#pragma once

/**
 * A Move in the game of KnightThrough.
 * A Move consists of:
 * - An int representing where we came from
 * - An int representing where we went to
 * - A bool indicating whether we captured an enemy piece on the location we went to
 */
struct Move
{
	/** The board location we came from */
	int from;
	/** The board location we went to */
	int to;
	/** True iff there was an enemy piece on the location we went to */
	bool captured;

	Move(int from, int to, bool captured);

	/** Overloaded == operator. Considers two objects to be equal iff all fields are equal */
	inline bool operator==(const Move& other) const
	{
		return ((from == other.from) && (to == other.to) && (captured == other.captured));
	}
};

static Move INVALID_MOVE = Move(-1, -1, false);