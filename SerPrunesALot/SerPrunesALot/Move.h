#pragma once

#include "BoardLocation.h"

/**
 * A Move in the game of KnightThrough.
 * A Move consists of:
 * - A BoardLocation representing where we came from
 * - A BoardLocation representing where we went to
 * - A bool indicating whether we captured an enemy piece on the location we went to
 */
struct Move
{
	/** The BoardLocation we came from */
	BoardLocation from;
	/** The BoardLocation we went to */
	BoardLocation to;
	/** True iff there was an enemy piece on the location we went to */
	bool captured;

	Move(BoardLocation from, BoardLocation to, bool captured);

	/** Overloaded == operator. Considers two objects to be equal iff all fields are equal */
	inline bool operator==(const Move& other) const
	{
		return ((from == other.from) && (to == other.to) && (captured == other.captured));
	}
};

static Move INVALID_MOVE = Move(BoardLocation(-1, -1), BoardLocation(-1, -1), false);