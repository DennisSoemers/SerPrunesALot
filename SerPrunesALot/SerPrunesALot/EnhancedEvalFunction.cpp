#include <algorithm>

#include "EnhancedEvalFunction.h"
#include "Logger.h"
#include "MathConstants.h"
#include "MoveOrdering.h"
#include "Timer.hpp"

/**
* The evaluation corresponding to a won game.
* Should be a non-tight upper bound on values the evaluation function can return in non-terminal game states
*/
#define WIN_EVALUATION 2000

/**
* The depth to which the engine should search the game tree.
*/
#define SEARCH_DEPTH 7

EnhancedEvalFunction::EnhancedEvalFunction() : transpositionTable(), lastRootEvaluation(0), totalNodesVisited(0), totalTimeSpent(0.0), turnsPlayed(0)
{}

Move EnhancedEvalFunction::chooseMove(GameState& gameState)
{
	transpositionTable.clear();	// clean up data from previous searches

#ifdef GATHER_STATISTICS
	nodesVisited = 0;
	Timer timer;
	timer.start();
	Move moveToPlay = startAlphaBetaTT(gameState, SEARCH_DEPTH);
	timer.stop();

#ifdef LOG_STATS_PER_TURN
	if (gameState.getCurrentPlayer() == EPlayerColors::Type::BLACK_PLAYER)
	{
		LOG_MESSAGE(StringBuilder() << "Enhanced Eval Function engine searching move for Black Player")
	}
	else
	{
		LOG_MESSAGE(StringBuilder() << "Enhanced Eval Function engine searching move for White Player")
	}

	LOG_MESSAGE(StringBuilder() << "Search depth:					" << SEARCH_DEPTH)
	LOG_MESSAGE(StringBuilder() << "Number of nodes visited:			" << nodesVisited)
	LOG_MESSAGE(StringBuilder() << "Time spent:					" << timer.getElapsedTimeInMilliSec() << " ms")
	LOG_MESSAGE(StringBuilder() << "% of Transposition Table entries used:		" << ((double)transpositionTable.getNumEntriesUsed() / (TRANSPOSITION_TABLE_NUM_ENTRIES * 2.0)))
	LOG_MESSAGE(StringBuilder() << "% of Transposition Table entries replaced:	" << ((double)transpositionTable.getNumReplacementsRequired() / (TRANSPOSITION_TABLE_NUM_ENTRIES * 2.0)))
	LOG_MESSAGE("")
#endif // LOG_STATS_PER_TURN

#ifdef LOG_STATS_END_OF_MATCH
	totalNodesVisited += nodesVisited;
	totalTimeSpent += timer.getElapsedTimeInMilliSec();
	++turnsPlayed;
#endif // LOG_STATS_END_OF_MATCH

	return moveToPlay;
#else
	return startAlphaBetaTT(gameState, SEARCH_DEPTH);
#endif // GATHER_STATISTICS
}

int EnhancedEvalFunction::alphaBetaTT(GameState& gameState, int depth, int alpha, int beta)
{
#ifdef GATHER_STATISTICS
	++nodesVisited;
#endif // GATHER_STATISTICS

	int originalAlpha = alpha;
	uint64_t zobrist = gameState.getZobrist();
	const TableData& tableData = transpositionTable.retrieve(zobrist);
	// true iff relevant data was retrieved from the Transposition Table
	bool tableDataValid = tableData.isValid();

#ifdef VERIFY_MOVE_LEGALITY
	if (tableDataValid && !gameState.isMoveLegal(tableData.bestMove))
	{
		LOG_ERROR("ERROR: table data contains invalid move in AlphaBetaTT::alphaBetaTT()")
		tableDataValid = false;
	}
#endif

	if (tableDataValid)
	{
		if (tableData.depth >= depth)	// ensure table stored in data resulted from a deep enough search
		{
			if (tableData.valueType == EValue::Type::REAL)
			{
				return tableData.value;
			}
			else if (tableData.valueType == EValue::Type::LOWER_BOUND)
			{
				alpha = std::max(alpha, tableData.value);
			}
			else if (tableData.valueType == EValue::Type::UPPER_BOUND)
			{
				beta = std::min(beta, tableData.value);
			}

			if (alpha >= beta)
			{
				return tableData.value;
			}
		}
	}

	EPlayerColors::Type winner = gameState.getWinner();

	// stop search if we reached max depth or have found a winner
	if (depth == 0 || winner != EPlayerColors::Type::NOTHING)
	{
		return evaluate(gameState, winner);
	}

	std::vector<Move> moves = gameState.generateAllMoves();
	if (tableDataValid)		// order moves such that best move according to Transposition Table is explored first
	{
		MoveOrdering::orderMoves(moves, tableData.bestMove);
	}

	int score = MathConstants::LOW_ENOUGH_INT;
	Move& bestMove = moves[0];

	int numMoves = moves.size();
	for (int i = 0; i < numMoves; ++i)
	{
		const Move& m = moves[i];											// select move
		gameState.applyMove(m);												// apply move
		transpositionTable.prefetch(gameState.getZobrist());				// prefetch transposition table data
		int value = -alphaBetaTT(gameState, depth - 1, -beta, -alpha);		// continue searching
		gameState.undoMove(m);												// finished searching this subtree, so undo the move

		if (value > score)		// new best move found
		{
			score = value;
			bestMove = m;
		}
		if (score > alpha)
		{
			alpha = score;
		}
		if (score >= beta)
		{
			break;
		}
	}

	// Store data in Transposition Table
	if (score <= originalAlpha)		// found upper bound
	{
		transpositionTable.storeData(bestMove, zobrist, score, EValue::Type::UPPER_BOUND, depth);
	}
	else if (score >= beta)			// found lower bound
	{
		transpositionTable.storeData(bestMove, zobrist, score, EValue::Type::LOWER_BOUND, depth);
	}
	else							// found exact value
	{
		transpositionTable.storeData(bestMove, zobrist, score, EValue::Type::REAL, depth);
	}

	return score;
}

int EnhancedEvalFunction::evaluate(const GameState& gameState) const
{
	return evaluate(gameState, gameState.getWinner());
}

int EnhancedEvalFunction::evaluate(const GameState& gameState, EPlayerColors::Type winner) const
{
	EPlayerColors::Type evaluatingPlayer = gameState.getCurrentPlayer();

	if (winner == evaluatingPlayer)						// evaluating player won
	{
		return WIN_EVALUATION;
	}
	else if (winner != EPlayerColors::Type::NOTHING)	// opponent won
	{
		return -WIN_EVALUATION;
	}

	// at this point in code, compute evaluation from white's perspective
	// at the end, before returning, negate if black is evaluating

	// simple material difference, weight = 100, range = [-1600, 1600]
	int materialDifference = 100 * (gameState.getNumWhiteKnights() - gameState.getNumBlackKnights());

	// progression = difference in furthest moved knight, weight = 35, range = [-210, 210] (because max advantage = 6)
	int progression = 0;

	/**
	 * controlled progression = difference in furthest square that we can also attack again more often than enemy can
	 * weight = 1, range = [-4, 4] (because max advantage = 4)
	 */
	int controlledProgression = 0;

	const std::vector<BoardLocation>& blackKnights = gameState.getBlackKnights();
	const std::vector<BoardLocation>& whiteKnights = gameState.getWhiteKnights();

	// give every player default controlled progression of 2, so we only reward controlled movement further than 2
	int blackProgression = 0;
	int whiteProgression = 0;
	int blackControlledProgression = 2;
	int whiteControlledProgression = 2;

	for (const BoardLocation& knight : blackKnights)
	{
		int distance = knight.y;

		if (distance > blackProgression)
		{
			blackProgression = distance;
		}

		if (distance > blackControlledProgression)
		{
			int numAttackers = gameState.getNumAttackers(knight, EPlayerColors::Type::WHITE_PLAYER);

			if (evaluatingPlayer == EPlayerColors::Type::BLACK_PLAYER)	// black player is to move, so a single white attacker isn't enough
			{
				--numAttackers;
			}

			if (numAttackers <= 0)	
			{
				blackControlledProgression = distance;		// black is completely safe here
			}
			else if (numAttackers <= gameState.getNumAttackers(knight, EPlayerColors::Type::BLACK_PLAYER))
			{
				blackControlledProgression = distance;		// black has enough back-up
			}
		}
	}

	for (const BoardLocation& knight : whiteKnights)
	{
		int distance = BOARD_HEIGHT - 1 - knight.y;

		if (distance > whiteProgression)
		{
			whiteProgression = distance;
		}
		
		if (distance > whiteControlledProgression)
		{
			int numAttackers = gameState.getNumAttackers(knight, EPlayerColors::Type::BLACK_PLAYER);

			if (evaluatingPlayer == EPlayerColors::Type::WHITE_PLAYER)	// white player is to move, so a single black attacker isn't enough
			{
				--numAttackers;
			}

			if (numAttackers <= 0)
			{
				whiteControlledProgression = distance;		// white is completely safe here
			}
			else if (numAttackers <= gameState.getNumAttackers(knight, EPlayerColors::Type::WHITE_PLAYER))
			{
				whiteControlledProgression = distance;		// white has enough back-up
			}
		}
	}

	progression = 35 * (whiteProgression - blackProgression);
	controlledProgression = 1 * (whiteControlledProgression - blackControlledProgression);

	// compute final score
	int score = materialDifference + progression + controlledProgression;

	// negate score in case we're black, since so far we assumed we're white
	if (evaluatingPlayer == EPlayerColors::Type::BLACK_PLAYER)
	{
		score = -score;
	}

	return score;
}

Move EnhancedEvalFunction::startAlphaBetaTT(GameState& gameState, int depth)
{
	int score = MathConstants::LOW_ENOUGH_INT;
	int alpha = MathConstants::LOW_ENOUGH_INT;
	int beta = MathConstants::LARGE_ENOUGH_INT;

	EPlayerColors::Type winner = gameState.getWinner();

	// stop search if we reached max depth or have found a winner
	if (depth == 0 || winner != EPlayerColors::Type::NOTHING)
	{
		return INVALID_MOVE;		// can't return any normal move if game already ended
	}

	std::vector<Move> moves = gameState.generateAllMoves();
	Move& bestMove = moves[0];

	for (int i = 0; i < moves.size(); ++i)
	{
		const Move& m = moves[i];											// select move
		gameState.applyMove(m);												// apply move
		transpositionTable.prefetch(gameState.getZobrist());				// prefetch transposition table data
		int value = -alphaBetaTT(gameState, depth - 1, -beta, -alpha);		// continue searching
		gameState.undoMove(m);												// finished searching this subtree, so undo the move

		if (value > score)		// new best move found
		{
			score = value;
			bestMove = m;
		}
		if (score > alpha)
		{
			alpha = score;
		}
		if (score >= beta)
		{
			break;
		}
	}

	lastRootEvaluation = score;

	return bestMove;
}

int EnhancedEvalFunction::getRootEvaluation()
{
	return lastRootEvaluation;
}

int EnhancedEvalFunction::getWinEvaluation()
{
	return WIN_EVALUATION;
}

void EnhancedEvalFunction::logEndOfMatchStats()
{
#ifdef LOG_STATS_END_OF_MATCH
	LOG_MESSAGE("Enhanced Eval Function engine END OF GAME stats:")
	LOG_MESSAGE(StringBuilder() << "Search depth:					" << SEARCH_DEPTH)
	LOG_MESSAGE(StringBuilder() << "Number of nodes visited:			" << totalNodesVisited)
	LOG_MESSAGE(StringBuilder() << "Time spent:					" << totalTimeSpent << " ms")
	LOG_MESSAGE("")
#endif // LOG_STATS_END_OF_MATCH
}