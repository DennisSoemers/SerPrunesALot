#include <algorithm>

#include "AlphaBetaTT.h"
#include "Logger.h"
#include "MathConstants.h"
#include "Timer.hpp"

/**
* The evaluation corresponding to a won game.
* Should be a non-tight upper bound on values the evaluation function can return in non-terminal game states
*/
#define WIN_EVALUATION 20

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
	const TableData& tableData = transpositionTable.retrieve(gameState.getZobrist());
	// true iff relevant data was retrieved from the Transposition Table
	bool tableDataValid = tableData.isValid();

#ifdef VERIFY_MOVE_LEGALITY
	if (tableDataValid && !gameState.isMoveLegal(tableData.bestMove))
	{
		//LOG_ERROR("ERROR: table data contains invalid move in AlphaBetaTT::alphaBetaTT()")
		//LOG_ERROR(StringBuilder() << "Captured = " << tableData.bestMove.captured)
		//LOG_ERROR(StringBuilder() << "From = " << tableData.bestMove.from.x << ", " << tableData.bestMove.from.y)
		//LOG_ERROR(StringBuilder() << "To = " << tableData.bestMove.to.x << ", " << tableData.bestMove.to.y)
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
	int score = MathConstants::LOW_ENOUGH_INT;
	Move bestMove = moves.at(0);

	for (int i = 0; i < moves.size(); ++i)
	{
		Move m = moves[i];													// select move
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
	}

	// Store data in Transposition Table
	if (score <= originalAlpha)		// found upper bound
	{
		transpositionTable.storeData(bestMove, gameState.getZobrist(), score, EValue::Type::UPPER_BOUND, depth);
	}
	else if (score >= beta)			// found lower bound
	{
		transpositionTable.storeData(bestMove, gameState.getZobrist(), score, EValue::Type::LOWER_BOUND, depth);
	}
	else							// found exact value
	{
		transpositionTable.storeData(bestMove, gameState.getZobrist(), score, EValue::Type::REAL, depth);
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
	int materialDifference = gameState.getNumWhiteKnights() - gameState.getNumBlackKnights();

	if (evaluatingPlayer == EPlayerColors::Type::BLACK_PLAYER)
	{
		materialDifference = -materialDifference;
	}

	return materialDifference;
}

Move AlphaBetaTT::startAlphaBetaTT(GameState& gameState, int depth)
{
	int score = MathConstants::LOW_ENOUGH_INT;
	int alpha = MathConstants::LOW_ENOUGH_INT;
	int beta = MathConstants::LARGE_ENOUGH_INT;
	int originalAlpha = alpha;

	const TableData& tableData = transpositionTable.retrieve(gameState.getZobrist());
	// true iff relevant data was retrieved from the Transposition Table
	bool tableDataValid = tableData.isValid();

#ifdef VERIFY_MOVE_LEGALITY
	if (tableDataValid && !gameState.isMoveLegal(tableData.bestMove))
	{
		LOG_ERROR("ERROR: table data contains invalid move in AlphaBetaTT::startAlphaBetaTT()")
		tableDataValid = false;
	}
#endif

	if (tableDataValid)
	{
		if (tableData.depth >= depth)	// ensure table stored in data resulted from a deep enough search
		{
			if (tableData.valueType != EValue::Type::UPPER_BOUND)		// means it's either real or lower bound
			{
				if (tableData.value >= beta)
				{
					return tableData.bestMove;
				}

				alpha = std::max(alpha, tableData.value);
			}

			if (tableData.valueType != EValue::Type::LOWER_BOUND)		// means it's either real or upper bound
			{
				if (tableData.value <= alpha)
				{
					return tableData.bestMove;
				}

				beta = std::min(beta, tableData.value);
			}
		}
	}

	EPlayerColors::Type winner = gameState.getWinner();

	// stop search if we reached max depth or have found a winner
	if (depth == 0 || winner != EPlayerColors::Type::NOTHING)
	{
		return INVALID_MOVE;		// can't return any normal move if game already ended
	}

	std::vector<Move> moves = gameState.generateAllMoves();
	Move bestMove = moves.at(0);

	for (int i = 0; i < moves.size(); ++i)
	{
		Move m = moves[i];													// select move
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
	}

	// Store data in Transposition Table
	if (score <= originalAlpha)		// found upper bound
	{
		transpositionTable.storeData(bestMove, gameState.getZobrist(), score, EValue::Type::UPPER_BOUND, depth);
	}
	else if (score >= beta)			// found lower bound
	{
		transpositionTable.storeData(bestMove, gameState.getZobrist(), score, EValue::Type::LOWER_BOUND, depth);
	}
	else							// found exact value
	{
		transpositionTable.storeData(bestMove, gameState.getZobrist(), score, EValue::Type::REAL, depth);
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