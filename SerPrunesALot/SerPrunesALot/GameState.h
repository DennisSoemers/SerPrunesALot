#pragma once

#include <inttypes.h>
#include <vector>

#include "GameConstants.h"
#include "Move.h"
#include "TranspositionTable.h"

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
 * Defines a state of the game
 */
class GameState
{
public:
	GameState();
	~GameState();

	/**
	 * Applies the given move to this game state.
	 *
	 * Does not perform any kinds of safety checks!
	 * Assumes that the given move is legal, and that the current player is executing this move. 
	 * Make sure only to call this method with proper input!
	 */
	void applyMove(const Move& move);

	/**
	 * Tests whether it is possible to move from the ''from'' location to the ''to'' location
	 * Does NOT test whether the corresponding player actually is the player that can currently move.
	 */
	bool canMove(int from, int to) const;

	/**
	 * Same as canMove(BoardLocation from, BoardLocation to), but with an extra ''player'' argument.
	 * This argument should contain the player that is trying to make the move.
	 *
	 * This value is assumed to be equal to the value contained in BoardLocation ''from'' in this game state!
	 * If that is NOT the case, the method can return an incorrect value.
	 *
	 * Useful to use instead of the method without the player argument if the player argument is already known
	 * from the calling site and therefore the method no longer needs to obtain this information
	 */
	bool canMove(int from, int to, EPlayerColors::Type player) const;

	/** 
	 * Generates a vector with all legal moves from the ''from'' location. 
	 * Does NOT test whether the player on the ''from'' location actually is the current player 
	 */
	std::vector<Move> generateMoves(int from) const;

	/** Returns the bitboard corresponding to the given player */
	uint64_t getBitboard(EPlayerColors::Type player) const;
	/** Returns an EPlayerColors::Type indicating which player is the current player */
	EPlayerColors::Type getCurrentPlayer() const;
	/** Returns the board locations that the given player can move to from the given square */
	static const std::vector<int>& getMoveTargets(int location, EPlayerColors::Type color);
	/** Returns the number of knights of the given color that could attack a given square (not taking into account whether it's occupied or not) */
	//int getNumAttackers(const int location, EPlayerColors::Type attackersColor) const;
	/** Returns the number of knights that the black player has */
	int getNumBlackKnights() const;
	/** Returns the number of knights that the white player has */
	int getNumWhiteKnights() const;
	/** Returns an EPlayerColors::Type indicating what (if anything) is occupying a given BoardLocation */
	EPlayerColors::Type getOccupier(int location) const;
	/** Given a player's color, returns the color of the opponent */
	EPlayerColors::Type getOpponentColor(EPlayerColors::Type color) const;
	/** Returns the color of the player that won the game. Returns EPlayerColors::Type::NOTHING if the game didn't end yet */
	EPlayerColors::Type getWinner() const;
	/** Returns the Zobrist Hash Value of the current game state */
	uint64_t getZobrist() const;

	/** Returns true iff the given move is legal in the current game state */
	bool isMoveLegal(const Move& move) const;

	/** Resets the game state to the starting setup */
	void reset();

	/** Changes who the current player is */
	void switchCurrentPlayer();

	/**
	 * Reverts game state to the way it was before applying the given move
	 *
	 * Assumes that currentPlayer was already switched back to the player that originally made the move!
	 */
	void undoMove(const Move& move);

private:
	/** Matrix of random numbers corresponding to board locations and player colors. Used for computing Zobrist Hash Values*/
	static std::vector<std::vector<uint64_t>> zobristRandomNums;

	/** Pre-computed table of move targets for the Black Player */
	static std::vector<std::vector<int>> moveTargetsBlack;
	/** Pre-computed table of move targets for the White Player */
	static std::vector<std::vector<int>> moveTargetsWhite;

	/** Bitboard of black pieces */
	int64_t blackBitboard;
	/** Bitboard of white pieces */
	int64_t whiteBitboard;

	/** The Zobrist Hash Value of this game state */
	uint64_t zobristHash;

	/** A single special random number that is XORd with the zobrist hash every time the turn switches, to indicate who the current player is */
	uint64_t zobristPlayerNum;

	/** The player whose turn it is */
	EPlayerColors::Type currentPlayer;

	/** The number of black knights remaining in this state */
	int numBlackKnights;
	/** The number of white knights remaining in this state */
	int numWhiteKnights;

	// Functions to initialize static move target tables
	static std::vector<std::vector<int>> precomputeMoveTargetsBlack();
	static std::vector<std::vector<int>> precomputeMoveTargetsWhite();

	// don't want accidental copying of game states
	GameState(const GameState&);
	GameState& operator=(const GameState&);
};