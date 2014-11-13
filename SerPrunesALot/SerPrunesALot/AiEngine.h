#pragma once

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
};