#pragma once

#include "Options.h"

#include "GameState.h"
#include "Move.h"

/**
 * Interface to an AI Engine class. 
 */
class AiEngine
{
public:
	/**
	 * Virtual method to choose a move given the current Game State.
	 * Contains no default implementation. MUST be implemented by subclasses!
	 *
	 * The current Game State is passed by reference, and is expected not
	 * to be modified by the method when it returns. However, the method
	 * IS allowed to modify the gameState object during it's search, as long
	 * as it also modifies any changes back.
	 *
	 * Because of this, the gameState object is NOT passed by as a const reference,
	 * but simply as a reference.
	 */
	virtual Move chooseMove(GameState& gameState) = 0;

	/**
	 * Virtual method that should be implemented to return the evaluation associated
	 * with a won game.
	 */
	virtual int getWinEvaluation() = 0;

	/** 
	 * Virtual method that should be implemented to return the evaluation of the
	 * root node during the last time the engine was asked to choose a move.
	 */
	virtual int getRootEvaluation() = 0;

	/** Logs statistics gathered by the AI engine at the end of the match */
	virtual void logEndOfMatchStats() = 0;
};