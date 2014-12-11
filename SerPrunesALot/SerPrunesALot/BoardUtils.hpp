#pragma once

#include "GameConstants.h"

/**
 * Utility methods to make dealing with board locations compactly represented as a single int
 * less annoying.
 *
 * The functions are forced to be inlined, so they can safely be used for improved code readability
 * without a loss of performance.
 */
namespace BoardUtils
{
	/** Returns true iff the given integer represents a valid board location */
	__forceinline bool isValid(int location)
	{
		return ((unsigned int) location) < 64;
	}

	/** Returns the board index corresponding to the given x and y coordinates */
	__forceinline int coordsToIndex(int x, int y)
	{
		return y * BOARD_HEIGHT + x;
	}

	/** Returns the x-coordinate of the given location. Assumes leftmost column = 0, rightmost column = 7 */
	__forceinline int x(int location)
	{
		return location % BOARD_WIDTH;
	}

	/** Returns the y-coordinate of the given location. Assumes top row = 0, bottom row = 7 */
	__forceinline int y(int location)
	{
		return location / BOARD_HEIGHT;
	}
}