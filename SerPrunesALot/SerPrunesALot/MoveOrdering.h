#pragma once

#include <vector>

#include "Move.h"

/**
 * Methods to order a given vector of moves
 */
namespace MoveOrdering
{
	/**
	 * Re-orders the given vector of moves by ensuring that ''bestMove'' ends up in the front
	 * of the vector.
	 *
	 * Example use case: try the ''bestMove'' obtained from Transposition Table first
	 */
	void orderMoves(std::vector<Move>& moves, const Move& bestMove);
}