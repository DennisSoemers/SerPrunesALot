#pragma once

#include <vector>

#include "BoardLocation.h"
#include "GameConstants.h"
#include "Player.h"

/**
 * Defines a state of the game
 */
class GameState
{
public:
	GameState();
	~GameState();

	/** Returns an EPlayerColors::Type indicating what (if anything) is occupying a given BoardLocation */
	EPlayerColors::Type getOccupier(BoardLocation location) const;

	/** Resets the game state to the starting setup */
	void reset();

private:
	/** Matrix of EPlayerColors::Types encoding the board. */
	std::vector<std::vector<EPlayerColors::Type>> board;

	/** The black player */
	Player blackPlayer;
	/** The white player */
	Player whitePlayer;

	/** The player whose turn it is */
	EPlayerColors::Type currentPlayer;

	// don't want accidental copying of game states
	GameState(const GameState&);
	GameState& operator=(const GameState&);
};