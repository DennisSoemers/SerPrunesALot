#pragma once

#include <vector>

#include "BoardLocation.h"

/** Possible colors that players can have */
namespace EPlayerColors
{
	enum Type
	{
		NOTHING,

		BLACK_PLAYER,
		WHITE_PLAYER,

		NUM_PLAYER_COLORS
	};
}

/**
 * Class representing a single Player in a game of KnightThrough
 */
class Player
{
public:
	Player(EPlayerColors::Type color);
	~Player();

	/** Gives this player a new knight at a given board location */
	void addKnight(BoardLocation boardLoc);

	/** 
	 * Returns the vector of locations where this player has knights. 
	 *
	 * NOTE: returned by reference, NOT const!
	 */
	std::vector<BoardLocation>& getKnightLocations();

	/** Returns the vector of locations where this player has knights by const reference */
	const std::vector<BoardLocation>& getKnightLocations() const;

	/** Returns the number of knights that this player has */
	int getNumKnights() const;

	/** Clears this player's vector of knight locations */
	void removeAllKnights();
	/** Removes a knight at a given board location from this player */
	void removeKnight(BoardLocation boardLoc);

private:
	/** Vector containing the locations where this player has knights */
	std::vector<BoardLocation> knightLocations;

	/** This player's color */
	EPlayerColors::Type color;
};