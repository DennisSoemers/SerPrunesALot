#include "BasicAlphaBeta.h"
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
#define SEARCH_DEPTH 6

BasicAlphaBeta::BasicAlphaBeta() : lastRootEvaluation(0), totalNodesVisited(0), totalTimeSpent(0.0), turnsPlayed(0)
{}

Move BasicAlphaBeta::chooseMove(GameState& gameState)
{
#ifdef GATHER_STATISTICS
	nodesVisited = 0;
	Timer timer;
	timer.start();
	Move moveToPlay = startAlphaBeta(gameState, SEARCH_DEPTH);
	timer.stop();

#ifdef LOG_STATS_PER_TURN
	if (gameState.getCurrentPlayer() == EPlayerColors::Type::BLACK_PLAYER)
	{
		LOG_MESSAGE(StringBuilder() << "Basic Alpha Beta engine searching move for Black Player")
	}
	else
	{
		LOG_MESSAGE(StringBuilder() << "Basic Alpha Beta engine searching move for White Player")
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
	return startAlphaBeta(gameState, SEARCH_DEPTH);
#endif // GATHER_STATISTICS
}

int BasicAlphaBeta::alphaBeta(GameState& gameState, int depth, int alpha, int beta)
{
#ifdef GATHER_STATISTICS
	++nodesVisited;
#endif // GATHER_STATISTICS

	EPlayerColors::Type winner = gameState.getWinner();

	// stop search if we reached max depth or have found a winner
	if (depth == 0 || winner != EPlayerColors::Type::NOTHING)
	{
		return evaluate(gameState, winner);
	}

	std::vector<Move> moves = gameState.generateAllMoves();
	int score = MathConstants::LOW_ENOUGH_INT;

	for (int i = 0; i < moves.size(); ++i)
	{
		Move m = moves[i];													// select move
		gameState.applyMove(m);												// apply move
		int value = -alphaBeta(gameState, depth - 1, -beta, -alpha);		// continue searching
		gameState.undoMove(m);												// finished searching this subtree, so undo the move

		if (value > score)		// new best move found
		{
			score = value;
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

	return score;
}

int BasicAlphaBeta::evaluate(const GameState& gameState) const
{
	return evaluate(gameState, gameState.getWinner());
}

int BasicAlphaBeta::evaluate(const GameState& gameState, EPlayerColors::Type winner) const
{
	EPlayerColors::Type evaluatingPlayer = gameState.getCurrentPlayer();

	if (winner == evaluatingPlayer)						// evaluating player won
	{
		return WIN_EVALUATION;
	}
	else if (winner != EPlayerColors::Type::NOTHING)	// opponent won
	{
		return - WIN_EVALUATION;
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

Move BasicAlphaBeta::startAlphaBeta(GameState& gameState, int depth)
{
	EPlayerColors::Type winner = gameState.getWinner();

	// stop search if we reached max depth or have found a winner
	if (depth == 0 || winner != EPlayerColors::Type::NOTHING)
	{
		return INVALID_MOVE;		// can't return any normal move if game already ended
	}

	std::vector<Move> moves = gameState.generateAllMoves();
	Move bestMove = moves.at(0);
	int score = MathConstants::LOW_ENOUGH_INT;
	int alpha = MathConstants::LOW_ENOUGH_INT;
	int beta = MathConstants::LARGE_ENOUGH_INT;

	for (int i = 0; i < moves.size(); ++i)
	{
		Move m = moves[i];													// select move
		gameState.applyMove(m);												// apply move
		int value = -alphaBeta(gameState, depth - 1, -beta, -alpha);		// continue searching
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

int BasicAlphaBeta::getRootEvaluation()
{
	return lastRootEvaluation;
}

int BasicAlphaBeta::getWinEvaluation()
{
	return WIN_EVALUATION;
}

void BasicAlphaBeta::logEndOfMatchStats()
{
#ifdef LOG_STATS_END_OF_MATCH
	LOG_MESSAGE("Basic Alpha Beta engine END OF GAME stats:")
	LOG_MESSAGE(StringBuilder() << "Search depth:					" << SEARCH_DEPTH)
	LOG_MESSAGE(StringBuilder() << "Number of nodes visited:			" << totalNodesVisited)
	LOG_MESSAGE(StringBuilder() << "Time spent:					" << totalTimeSpent << " ms")
	LOG_MESSAGE("")
#endif // LOG_STATS_END_OF_MATCH
}