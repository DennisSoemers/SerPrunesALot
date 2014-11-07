#pragma once

/** 
 * A location on the board, indicated by an x and a y coordinate. 
 * Top-left corner of board has (x, y) = (0, 0).
 * Bottom-right corner of board has (x, y) = (BOARD_WIDTH - 1, BOARD_HEIGHT - 1)
 */
struct BoardLocation
{
public:
	/** The location's x-coordinate on the board */
	int x;
	/** The location's y-coordinate on the board */
	int y;

	BoardLocation(int x, int y);

	/** Returns true iff this is a valid location on the game board */
	bool isValid() const;

	/** Overloaded == operator */
	inline bool operator==(const BoardLocation& other) const
	{
		return ((x == other.x) && (y == other.y));
	}
};