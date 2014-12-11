#pragma once

#include "Bitboards.hpp"
#include <inttypes.h>
#include <vector>

#include "GameState.h"
#include "Move.h"

/**
 * A Move Generator class.
 *
 * Objects of this class can be initialized with moves from Transposition Table and Killer Moves,
 * and then queried for moves. They will automatically return TT moves and Killer Moves first if provided, 
 * and only then actually generate the other moves if necessary.
 */
class MoveGenerator
{
public:
	/**
	 * Constructs a Move Generator
	 *
	 * playerColor = The color of the player to move
	 * playerBitboard = The bitboard of the player to move
	 * opponentBitboard = The bitboard of the opponent of the player to move
	 * transpositionMove = Best move according to Transposition Table
	 * killerMove1 = The first killer move
	 * killerMove2 = The second killer move
	 */
	MoveGenerator(EPlayerColors::Type playerColor, uint64_t playerBitboard, uint64_t opponentBitboard,
				  Move transpositionMove = INVALID_MOVE, Move killerMove1 = INVALID_MOVE, Move killerMove2 = INVALID_MOVE);

	/** Returns the next move. Returns INVALID_MOVE if there are no more moves */
	Move nextMove();

private:
	/** This vector will contain the moves when the generator creates them */
	std::vector<Move> moves;

	/** Best move according to TT */
	Move transpositionMove;
	/** First killer move */
	Move killerMove1;
	/** Second killer move */
	Move killerMove2;

	/** The bitboard corresponding to the player to move */
	const uint64_t playerBitboard;
	/** The bitboard corresponding to the opponent of the move */
	const uint64_t opponentBitboard;
	/** The color of the player to move */
	const EPlayerColors::Type playerColor;

	/** The index of the last move that was returned. */
	size_t moveIndex;

	/** True iff all moves have been generated (can still be false if first returning TT / Killer Moves) */
	bool generatedMoves;
};