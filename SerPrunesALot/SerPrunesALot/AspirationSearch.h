#pragma once

#include <inttypes.h>

#include "AiEngine.h"
#include "Timer.hpp"
#include "TranspositionTable.h"

/**
* Engine using Aspiration Search. Similar to Iterative Deepening, except for starting searches
* with a smaller window.
*/
class AspirationSearch : public AiEngine
{
public:
	AspirationSearch::AspirationSearch();

	virtual Move chooseMove(GameState& gameState);

	/** Returns the last depth that the algorithm managed to fully search */
	int getLastSearchDepth();
	/** Returns the number of seconds spent searching last time */
	double getSecondsSearched();

	virtual int getRootEvaluation();
	virtual int getWinEvaluation();
	virtual void logEndOfMatchStats();

private:
	/** The engine's Transposition Table */
	TranspositionTable transpositionTable;

	/** Table of killer moves */
	std::vector<std::vector<Move>> killerMoves;

	/** A clock used to avoid overshooting the allowed search time by too much */
	Timer clock;

	/** The evaluation of the root node during the last search */
	int lastRootEvaluation;

	/** The minimum amount of time in milliseconds that the algorithm will spend search */
	const int MIN_SEARCH_TIME_MS = 25000;
	/** The maximum amount of time in milliseconds that the algorithm will spend trying to complete the current search when over MIN_SEARCH_TIME_MS */
	const int MAX_EXTRA_SEARCH_TIME_MS = 5000;

	// variables used for gathering and logging statistics
	int nodesVisited;
	int64_t totalNodesVisited;
	double totalTimeSpent;
	int turnsPlayed;
	int searchDepth;

	/**
	* Continues alpha-beta search, given the game state, maximum search depth, and current alpha and beta values.
	* Returns the node's evaluation.
	*/
	int alphaBeta(GameState& gameState, int depth, int alpha, int beta);

	/**
	* Returns an evaluation of the given game state.
	*
	* Assumes that the game state should be evaluated from the perspective of the ''currentPlayer'' value in the game state.
	*/
	int evaluate(const GameState& gameState) const;

	/** Same as above, but requires passing an additional winner argument. Optimization if winner has already been determined in calling code */
	int evaluate(const GameState& gameState, EPlayerColors::Type winner) const;

	/**
	* Starts search, given the current game state, a maximum search depth, and a (potentially ordered) vector of moves available in the root.
	* Returns the best Move to play
	*/
	Move startAspirationSearch(GameState& gameState);
};