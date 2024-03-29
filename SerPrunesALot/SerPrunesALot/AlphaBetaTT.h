#pragma once

#include <inttypes.h>

#include "AiEngine.h"
#include "TranspositionTable.h"

/**
 * Similar to the BasicAlphaBeta engine, but enhanced to use a Transposition Table
 */
class AlphaBetaTT : public AiEngine
{
public:
	AlphaBetaTT::AlphaBetaTT();

	/** Uses the Alpha Beta algorithm with a Transposition Table to choose a move */
	virtual Move chooseMove(GameState& gameState);

	virtual int getRootEvaluation();
	virtual int getWinEvaluation();
	virtual void logEndOfMatchStats();

private:
	/** The engine's Transposition Table */
	TranspositionTable transpositionTable;

	/** The evaluation of the root node during the last search */
	int lastRootEvaluation;

	// variables used for gathering and logging statistics
	int nodesVisited;
	int64_t totalNodesVisited;
	double totalTimeSpent;
	int turnsPlayed;

	/**
	* Continues alpha-beta search, given the game state, maximum search depth, and current alpha and beta values.
	* Returns the node's evaluation.
	*/
	int alphaBetaTT(GameState& gameState, int depth, int alpha, int beta);

	/**
	* Returns an evaluation of the given game state.
	*
	* Assumes that the game state should be evaluated from the perspective of the ''currentPlayer'' value in the game state.
	*/
	int evaluate(const GameState& gameState) const;

	/** Same as above, but requires passing an additional winner argument. Optimization if winner has already been determined in calling code */
	int evaluate(const GameState& gameState, EPlayerColors::Type winner) const;

	/**
	* Starts alpha-beta search, given the current game state and a maximum search depth.
	* Returns the best Move to play
	*/
	Move startAlphaBetaTT(GameState& gameState, int depth);
};