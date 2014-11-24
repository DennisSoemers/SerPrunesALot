#include "Logger.h"
#include "MoveOrdering.h"

#include <algorithm>

void MoveOrdering::orderMoves(std::vector<Move>& moves, const Move& bestMove)
{
	for (size_t i = 0; i < moves.size(); ++i)
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