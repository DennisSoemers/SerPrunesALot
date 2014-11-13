#pragma once

#include "AiEngine.h"

class BasicAlphaBeta : public AiEngine
{
	/** Uses basic Alpha Beta algorithm to choose a move */
	virtual Move chooseMove(GameState& gameState);
};