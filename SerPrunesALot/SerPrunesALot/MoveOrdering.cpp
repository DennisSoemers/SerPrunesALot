#include "Logger.h"
#include "MoveOrdering.h"

#include <algorithm>

void MoveOrdering::orderMoves(std::vector<Move>& moves, const Move& bestMove)
{
	size_t numMoves = moves.size();
	for (size_t i = 0; i < numMoves; ++i)
	{
		if (moves[i] == bestMove)		// found the best move, so swap it with the start of the vector
		{
			std::swap(moves[0], moves[i]);
			return;
		}
	}

#ifdef ALLOW_LOGGING
	LOG_ERROR("MoveOrdering::orderMoves(std::vector<Move>&, const Move&) failed; best move not found in vector of moves!")
#endif
}

void MoveOrdering::orderMoves(std::vector<Move>& moves, const std::vector<int>& moveScores)
{
	// using the temporary pairs vector first instead of directly sorting because this way I can use
	// std::sort instead of implementing my own sorting algorithm

	size_t numMoves = moves.size();

	// create a new vector that will sort <Move, score> pairs
	std::vector<std::pair<Move, int>> pairs;
	pairs.reserve(numMoves);

	for (size_t i = 0; i < numMoves; ++i)
	{
		pairs.push_back(std::make_pair(moves[i], moveScores[i]));
	}

	// sort the vector of pairs
	std::sort(pairs.begin(), pairs.end(), [](std::pair<Move, int>& pair1, std::pair<Move, int>& pair2)
											{return (pair1.second < pair2.second); }					);

	// overwrite the objects in moves with the properly sorted versions
	for (size_t i = 0; i < numMoves; ++i)
	{
		moves[i] = pairs[i].first;
	}
}