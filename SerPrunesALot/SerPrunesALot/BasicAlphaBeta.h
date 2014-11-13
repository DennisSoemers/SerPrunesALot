#pragma once

#include "AiEngine.h"

class BasicAlphaBeta : public AiEngine
{
public:
	/** Uses basic Alpha Beta algorithm to choose a move */
	virtual Move chooseMove(GameState& gameState);

private:
	/**
	 * Continues alpha-beta search, given the game state, maximum search depth, and current alpha and beta values.
	 * Returns the node's evaluation.
	 */
	int alphaBeta(GameState& gameState, int depth, int alpha, int beta);

	/**
	 * Returns an evaluation of the given game state.
	 *
	 * Assumes that the game state should be evaluated from the perspective of the ''currentPlayer'' value in the game state.
	 * This means that a move should first be applied, then evaluated, and THEN the current player should be switched!
	 */
	int evaluate(const GameState& gameState) const;

	/** 
	 * Starts alpha-beta search, given the current game state and a maximum search depth.
	 * Returns the best Move to play
	 */
	Move startAlphaBeta(GameState& gameState, int depth);
};