#pragma once

#include <vector>

#include "Move.h"

/**
 * Methods to order a given vector of moves
 */
namespace MoveOrdering
{
	/**
	 * Re-orders the given vector of moves such that they are ordered from highest score to lowest
	 * score, where in the original input moveScores[i] represents the score of moves[i]
	 *
	 * Example use case: order the moves at the root node based on scores found with previous search in Iterative Deepening
	 */
	void orderMoves(std::vector<Move>& moves, const std::vector<int>& moveScores);
}