#include <algorithm>

#include "AlphaBetaTT.h"
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

AlphaBetaTT::AlphaBetaTT() : transpositionTable(), lastRootEvaluation(0), totalNodesVisited(0), totalTimeSpent(0.0), turnsPlayed(0)
{}

Move AlphaBetaTT::chooseMove(GameState& gameState)
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
		LOG_MESSAGE(StringBuilder() << "Alpha Beta with TT engine searching move for Black Player")
	}
	else
	{
		LOG_MESSAGE(StringBuilder() << "Alpha Beta with TT engine searching move for White Player")
	}

	LOG_MESSAGE(StringBuilder() << "Search depth:					" << SEARCH_DEPTH)
	LOG_MESSAGE(StringBuilder() << "Number of nodes visited:			" << nodesVisited)
	LOG_MESSAGE(StringBuilder() << "Time spent:					" << timer.getElapsedTimeInMilliSec() << " ms")
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

int AlphaBetaTT::alphaBetaTT(GameState& gameState, int depth, int alpha, int beta)
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

	EPlayerColors::Type currentPlayer = gameState.getCurrentPlayer();
	Move transpositionMove = (tableDataValid) ? tableData.bestMove : INVALID_MOVE;
	MoveGenerator moveGenerator(currentPlayer,
								gameState.getBitboard(currentPlayer),
								gameState.getBitboard(gameState.getOpponentColor(currentPlayer)),
								transpositionMove);

	int score = MathConstants::LOW_ENOUGH_INT;
	Move m = moveGenerator.nextMove();
	Move bestMove = m;

	while (!(m == INVALID_MOVE))
	{
		gameState.applyMove(m);												// apply move
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

		m = moveGenerator.nextMove();
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

int AlphaBetaTT::evaluate(const GameState& gameState) const
{
	return evaluate(gameState, gameState.getWinner());
}

int AlphaBetaTT::evaluate(const GameState& gameState, EPlayerColors::Type winner) const
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

	// simple material difference, range = [-1600, 1600]
	int materialDifference = 100 * (gameState.getNumWhiteKnights() - gameState.getNumBlackKnights());

	// progression = difference in furthest moved knight, weight = 35, range = [-210, 210] (because max advantage = 6)
	int progression = 0;

	uint64_t blackBitboard = gameState.getBitboard(EPlayerColors::Type::BLACK_PLAYER);
	uint64_t whiteBitboard = gameState.getBitboard(EPlayerColors::Type::BLACK_PLAYER);

	int blackProgression = 0;
	int whiteProgression = 0;

	if(blackBitboard & Bitboards::ROW_2)
	{
		blackProgression = 6;
	}
	else if(blackBitboard & Bitboards::ROW_3)
	{
		blackProgression = 5;
	}
	else if(blackBitboard & Bitboards::ROW_4)
	{
		blackProgression = 4;
	}
	else if(blackBitboard & Bitboards::ROW_5)
	{
		blackProgression = 3;
	}
	else if(blackBitboard & Bitboards::ROW_6)
	{
		blackProgression = 2;
	}
	else if(blackBitboard & Bitboards::ROW_7)
	{
		blackProgression = 1;
	}

	if(whiteBitboard & Bitboards::ROW_7)
	{
		whiteProgression = 6;
	}
	else if(whiteBitboard & Bitboards::ROW_6)
	{
		whiteProgression = 5;
	}
	else if(whiteBitboard & Bitboards::ROW_5)
	{
		whiteProgression = 4;
	}
	else if(whiteBitboard & Bitboards::ROW_4)
	{
		whiteProgression = 3;
	}
	else if(whiteBitboard & Bitboards::ROW_3)
	{
		whiteProgression = 2;
	}
	else if(whiteBitboard & Bitboards::ROW_2)
	{
		whiteProgression = 1;
	}

	progression = 35 * (whiteProgression - blackProgression);

	// compute final score
	int score = materialDifference + progression;

	// negate score in case we're black, since so far we assumed we're white
	if (evaluatingPlayer == EPlayerColors::Type::BLACK_PLAYER)
	{
		score = -score;
	}

	return score;
}

Move AlphaBetaTT::startAlphaBetaTT(GameState& gameState, int depth)
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

	EPlayerColors::Type currentPlayer = gameState.getCurrentPlayer();
	MoveGenerator moveGenerator(currentPlayer,
								gameState.getBitboard(currentPlayer),
								gameState.getBitboard(gameState.getOpponentColor(currentPlayer)));

	Move m = moveGenerator.nextMove();
	Move bestMove = m;

	while(!(m == INVALID_MOVE))
	{
		gameState.applyMove(m);												// apply move
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

		m = moveGenerator.nextMove();
	}

	lastRootEvaluation = score;

	return bestMove;
}

int AlphaBetaTT::getRootEvaluation()
{
	return lastRootEvaluation;
}

int AlphaBetaTT::getWinEvaluation()
{
	return WIN_EVALUATION;
}

void AlphaBetaTT::logEndOfMatchStats()
{
#ifdef LOG_STATS_END_OF_MATCH
	LOG_MESSAGE("Alpha Beta with TT engine END OF GAME stats:")
	LOG_MESSAGE(StringBuilder() << "Search depth:					" << SEARCH_DEPTH)
	LOG_MESSAGE(StringBuilder() << "Number of nodes visited:			" << totalNodesVisited)
	LOG_MESSAGE(StringBuilder() << "Time spent:					" << totalTimeSpent << " ms")
	LOG_MESSAGE("")
#endif // LOG_STATS_END_OF_MATCH
}