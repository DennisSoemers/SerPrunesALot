#define NOMINMAX

#include <algorithm>

#include "AspirationSearch.h"
#include "Logger.h"
#include "MathConstants.h"
#include "MoveOrdering.h"

/**
* The evaluation corresponding to a won game.
* Should be a non-tight upper bound on values the evaluation function can return in non-terminal game states
*/
#define WIN_EVALUATION 1900

AspirationSearch::AspirationSearch()
	: transpositionTable(),
	killerMoves(),
	clock(),
	lastRootEvaluation(0),
	totalNodesVisited(0),
	totalTimeSpent(0.0),
	turnsPlayed(0),
	searchDepth(0)
{
}

Move AspirationSearch::chooseMove(GameState& gameState)
{
	transpositionTable.clear();	// clean up data from previous searches

#ifdef GATHER_STATISTICS
	nodesVisited = 0;
	Timer timer;
	timer.start();
	Move moveToPlay = startAspirationSearch(gameState);
	timer.stop();

#ifdef LOG_STATS_PER_TURN
	if(gameState.getCurrentPlayer() == EPlayerColors::Type::BLACK_PLAYER)
	{
		LOG_MESSAGE(StringBuilder() << "Aspiration Search engine searching move for Black Player")
	}
	else
	{
		LOG_MESSAGE(StringBuilder() << "Aspiration Search engine searching move for White Player")
	}

	LOG_MESSAGE(StringBuilder() << "Search depth:					" << searchDepth)
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
	return startAspirationSearch(gameState, SEARCH_DEPTH);
#endif // GATHER_STATISTICS
}

int AspirationSearch::alphaBeta(GameState& gameState, int depth, int alpha, int beta)
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
	if(tableDataValid && !gameState.isMoveLegal(tableData.bestMove))
	{
		LOG_ERROR("ERROR: table data contains invalid move in AspirationSearch::alphaBetaTT")
		tableDataValid = false;
	}
#endif

	if(tableDataValid)
	{
		if(tableData.depth >= depth)	// ensure table stored in data resulted from a deep enough search
		{
			if(tableData.valueType == EValue::Type::REAL)
			{
				return tableData.value;
			}
			else if(tableData.valueType == EValue::Type::LOWER_BOUND)
			{
				alpha = std::max(alpha, tableData.value);
			}
			else if(tableData.valueType == EValue::Type::UPPER_BOUND)
			{
				beta = std::min(beta, tableData.value);
			}

			if(alpha >= beta)
			{
				return tableData.value;
			}
		}
	}

	EPlayerColors::Type winner = gameState.getWinner();

	// stop search if we reached max depth or have found a winner
	if(depth == 0 || winner != EPlayerColors::Type::NOTHING)
	{
		return evaluate(gameState, winner);
	}

	EPlayerColors::Type currentPlayer = gameState.getCurrentPlayer();
	Move transpositionMove = (tableDataValid) ? tableData.bestMove : INVALID_MOVE;

	Move killerMove1 = INVALID_MOVE;
	Move killerMove2 = INVALID_MOVE;

	if(killerMoves.size() > depth)	// may have killer moves stored
	{
		std::vector<Move>& currentDepthKillerMoves = killerMoves[depth];

		if(currentDepthKillerMoves.size() > 0)
		{
			killerMove1 = currentDepthKillerMoves[0];

			if(currentDepthKillerMoves.size() > 1)
			{
				killerMove2 = currentDepthKillerMoves[1];
			}
		}
	}

	MoveGenerator moveGenerator(currentPlayer,
								gameState.getBitboard(currentPlayer),
								gameState.getBitboard(gameState.getOpponentColor(currentPlayer)),
								transpositionMove, killerMove1, killerMove2);

	int score = MathConstants::LOW_ENOUGH_INT;
	Move m = moveGenerator.nextMove();
	Move bestMove = m;

	while(!(m == INVALID_MOVE))
	{
		gameState.applyMove(m);												// apply move
		transpositionTable.prefetch(gameState.getZobrist());				// prefetch transposition table data
		int value = -alphaBeta(gameState, depth - 1, -beta, -alpha);		// continue searching
		gameState.undoMove(m);												// finished searching this subtree, so undo the move

		if(clock.getElapsedTimeInMilliSec() >= MIN_SEARCH_TIME_MS + MAX_EXTRA_SEARCH_TIME_MS)		// exceeding time limit
		{
			return 0;
		}

		if(value > score)		// new best move found
		{
			score = value;
			bestMove = m;
		}
		if(score > alpha)
		{
			alpha = score;
		}
		if(score >= beta)
		{
			// pruning, store Killer Move
			while(depth >= killerMoves.size()) // don't have a vector of killer moves yet for this depth
			{
				std::vector<Move> currentDepthKillerMoves;
				currentDepthKillerMoves.reserve(2);		// 2 slots of Killer Moves
				killerMoves.push_back(currentDepthKillerMoves);
			}

			std::vector<Move>& currentDepthKillerMoves = killerMoves[depth];	// killer moves for this depth
			if(currentDepthKillerMoves.size() > 0)		// already have at least 1 killer move
			{
				if(currentDepthKillerMoves[0] == m)		// this killer move already stored in first slot
				{
					break;
				}

				if(currentDepthKillerMoves.size() > 1)	// already have 2 killer moves stored
				{
					if(currentDepthKillerMoves[1] == m)	// this killer move already stored in second slot
					{
						break;
					}

					currentDepthKillerMoves[0] = currentDepthKillerMoves[1];	// move second kill move to first slot
					currentDepthKillerMoves.pop_back();							// and then remove second (which is now also in first slot)
				}
			}

			currentDepthKillerMoves.push_back(m);							// and put the new kill move in second slot

			break;
		}

		m = moveGenerator.nextMove();
	}

	// Store data in Transposition Table
	if(score <= originalAlpha)		// found upper bound
	{
		transpositionTable.storeData(bestMove, zobrist, score, EValue::Type::UPPER_BOUND, depth);
	}
	else if(score >= beta)			// found lower bound
	{
		transpositionTable.storeData(bestMove, zobrist, score, EValue::Type::LOWER_BOUND, depth);
	}
	else							// found exact value
	{
		transpositionTable.storeData(bestMove, zobrist, score, EValue::Type::REAL, depth);
	}

	return score;
}

int AspirationSearch::evaluate(const GameState& gameState) const
{
	return evaluate(gameState, gameState.getWinner());
}

int AspirationSearch::evaluate(const GameState& gameState, EPlayerColors::Type winner) const
{
	EPlayerColors::Type evaluatingPlayer = gameState.getCurrentPlayer();

	if(winner == evaluatingPlayer)						// evaluating player won
	{
		return WIN_EVALUATION;
	}
	else if(winner != EPlayerColors::Type::NOTHING)	// opponent won
	{
		return -WIN_EVALUATION;
	}

	// at this point in code, compute evaluation from white's perspective
	// at the end, before returning, negate if black is evaluating

	// simple material difference, weight = 100, range = [-1600, 1600]
	int materialDifference = 100 * (gameState.getNumWhiteKnights() - gameState.getNumBlackKnights());

	// progression = difference in furthest moved knight, weight = 35, range = [-210, 210] (because max advantage = 6)
	int progression = 0;

	uint64_t blackBitboard = gameState.getBitboard(EPlayerColors::Type::BLACK_PLAYER);
	uint64_t whiteBitboard = gameState.getBitboard(EPlayerColors::Type::WHITE_PLAYER);

	// If black is to move next and already has a piece in the bottom danger zone, simply treat it as a win for black
	if(evaluatingPlayer == EPlayerColors::Type::BLACK_PLAYER && (blackBitboard & Bitboards::DANGER_ZONE_BOTTOM))
	{
		return WIN_EVALUATION;
	}	// and similar check for white
	else if(evaluatingPlayer == EPlayerColors::Type::WHITE_PLAYER && (whiteBitboard & Bitboards::DANGER_ZONE_TOP))
	{
		return WIN_EVALUATION;
	}

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
	if(evaluatingPlayer == EPlayerColors::Type::BLACK_PLAYER)
	{
		score = -score;
	}

	return score;
}

int AspirationSearch::getLastSearchDepth()
{
	return searchDepth;
}

double AspirationSearch::getSecondsSearched()
{
	return clock.getElapsedTimeInSec();
}

Move AspirationSearch::startAspirationSearch(GameState& gameState)
{
	clock.start();
	EPlayerColors::Type winner = gameState.getWinner();

	// stop search if we reached max depth or have found a winner
	if(winner != EPlayerColors::Type::NOTHING)
	{
		return INVALID_MOVE;		// can't return any normal move if game already ended
	}

	std::vector<Move> moves;				// will store all the moves in the root node, necessary for move ordering based on scores found in previous searches
	moves.reserve(16 * 4);

	EPlayerColors::Type currentPlayer = gameState.getCurrentPlayer();
	MoveGenerator moveGenerator(currentPlayer,
								gameState.getBitboard(currentPlayer),
								gameState.getBitboard(gameState.getOpponentColor(currentPlayer)));

	Move rootMove = moveGenerator.nextMove();

	while(!(rootMove == INVALID_MOVE))
	{
		moves.push_back(rootMove);
		rootMove = moveGenerator.nextMove();
	}

	std::vector<int> moveScores;			// will store the scores of the moves here, to use for sorting
	moveScores.reserve(moves.size());

	// best move found from a complete search (so not considering searches that were terminated early)
	Move bestMoveCompleteSearch = moves[0];

	int guess = lastRootEvaluation;		// start guess with the final root evaluation of our previous search
	int deltaGuess = 100;

	// compensate guess for odd-even effect
	if(searchDepth % 2 == 0)			// last search ended at even depth
	{
		guess += 141;
	}
	// no need to compensate if previous search ended at odd depth, since we're also starting at odd depth now

	searchDepth = 0;
	while(true)
	{
		++searchDepth;			// increment search depth for the new search
		killerMoves.clear();	// clear table of killer moves

		// ================= ALPHA BETA ALGORITHM STARTS HERE =================
		int score = MathConstants::LOW_ENOUGH_INT;
		int alpha = guess - deltaGuess;
		int beta = guess + deltaGuess;

		// best move for only this particular search
		Move bestMove = moves[0];

		for(int i = 0; i < moves.size(); ++i)
		{
			const Move& m = moves[i];											// select move
			gameState.applyMove(m);												// apply move
			transpositionTable.prefetch(gameState.getZobrist());				// prefetch transposition table data
			int value = -alphaBeta(gameState, searchDepth - 1, -beta, -alpha);	// continue searching
			gameState.undoMove(m);												// finished searching this subtree, so undo the move

			if(clock.getElapsedTimeInMilliSec() >= MIN_SEARCH_TIME_MS + MAX_EXTRA_SEARCH_TIME_MS)		// exceeding time limit
			{
				bestMove = INVALID_MOVE;
				break;
			}

			moveScores[i] = value;

			if(value > score)		// new best move found
			{
				score = value;
				bestMove = m;
			}
			if(score > alpha)
			{
				alpha = score;
			}
			if(score >= beta)
			{
				break;
			}
		}
		// =================  ALPHA BETA ALGORITHM RESTARTS IF ASPIRATION SEARCH GAVE INCORRECT RESULT  =================
		bool newSearchNeeded = false;

		if(score >= (guess + deltaGuess) && !(bestMove == INVALID_MOVE))
		{
			newSearchNeeded = true;
			alpha = score;
			beta = MathConstants::LARGE_ENOUGH_INT;
		}
		else if(score <= (guess - deltaGuess) && !(bestMove == INVALID_MOVE))
		{
			newSearchNeeded = true;
			alpha = MathConstants::LOW_ENOUGH_INT;
			beta = score;
		}

		if(newSearchNeeded)		// Aspiration Search failed us, re-start the entire thing
		{
			LOG_MESSAGE(StringBuilder() << ">>>>>>>>>>>>>>>> Aspiration Search required a new Search at depth = " << searchDepth << "! <<<<<<<<<<<<<<<<<<<")
			LOG_MESSAGE(StringBuilder() << "Window = [" << (guess - deltaGuess) << ", " << (guess + deltaGuess) << "]")
			score = MathConstants::LOW_ENOUGH_INT;

			// best move for only this particular search
			bestMove = moves[0];

			for(int i = 0; i < moves.size(); ++i)
			{
				const Move& m = moves[i];											// select move
				gameState.applyMove(m);												// apply move
				transpositionTable.prefetch(gameState.getZobrist());				// prefetch transposition table data
				int value = -alphaBeta(gameState, searchDepth - 1, -beta, -alpha);	// continue searching
				gameState.undoMove(m);												// finished searching this subtree, so undo the move

				if(clock.getElapsedTimeInMilliSec() >= MIN_SEARCH_TIME_MS + MAX_EXTRA_SEARCH_TIME_MS)		// exceeding time limit
				{
					bestMove = INVALID_MOVE;
					break;
				}

				moveScores[i] = value;

				if(value > score)		// new best move found
				{
					score = value;
					bestMove = m;
				}
				if(score > alpha)
				{
					alpha = score;
				}
				if(score >= beta)
				{
					break;
				}
			}

			LOG_MESSAGE(StringBuilder() << "True score = " << score)
		}
		// =================  ALPHA BETA ALGORITHM ENDS HERE  =================

		if(!(bestMove == INVALID_MOVE))	// managed to complete the search within time
		{
			lastRootEvaluation = score;

			if(score == WIN_EVALUATION)	// the search was enough to prove a win for us, so return best move of this latest search
			{
				return bestMove;
			}
			else if(score == -WIN_EVALUATION)	// the search proved a win for opponent, so return best move of the previous search
			{
				return bestMoveCompleteSearch;
			}

			// finished search, and didn't prove a win for either team, so save the new best result
			bestMoveCompleteSearch = bestMove;
		}
		else
		{
			--searchDepth;	// since last search was unsuccessful, decrement this so GUI doesn't lie to us
		}

		if(clock.getElapsedTimeInMilliSec() >= MIN_SEARCH_TIME_MS)		// exceeding time limit
		{
			clock.stop();
			return bestMoveCompleteSearch;
		}

		MoveOrdering::orderMoves(moves, moveScores);	// order moves for the next search

		// reset all scores to 0 before starting new search
		for(int i = 0; i < moveScores.size(); ++i)
		{
			moveScores[i] = 0;
		}

		// set our new guess for the next depth
		guess = score;

		if(searchDepth % 2 == 0)	// we've just done an even depth search, gonna do odd now, so compensate by increasing guess
		{
			guess += 141;
		}
		else						// we've just done an odd depth search, gonna do even now, so compensate by decreasing guess
		{
			guess -= 141;
		}
	}
}

int AspirationSearch::getRootEvaluation()
{
	return lastRootEvaluation;
}

int AspirationSearch::getWinEvaluation()
{
	return WIN_EVALUATION;
}

void AspirationSearch::logEndOfMatchStats()
{
#ifdef LOG_STATS_END_OF_MATCH
	LOG_MESSAGE("Aspiration Search engine END OF GAME stats:")
	LOG_MESSAGE(StringBuilder() << "Number of nodes visited:			" << totalNodesVisited)
	LOG_MESSAGE(StringBuilder() << "Time spent:					" << totalTimeSpent << " ms")
	LOG_MESSAGE("")
#endif // LOG_STATS_END_OF_MATCH
}