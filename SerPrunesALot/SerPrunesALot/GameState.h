#pragma once

#include <vector>

#include "BoardLocation.h"
#include "GameConstants.h"
#include "Move.h"
#include "Player.h"

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
	bool canMove(BoardLocation from, BoardLocation to) const;

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
	bool canMove(BoardLocation from, BoardLocation to, EPlayerColors::Type player) const;

	/**
	 * Generates a vector with ALL legal moves in the current game state.
	 */
	std::vector<Move> generateAllMoves() const;

	/** 
	 * Generates a vector with all legal moves from the ''from'' location. 
	 * Does NOT test whether the player on the ''from'' location actually is the current player 
	 */
	std::vector<Move> generateMoves(BoardLocation from) const;

	/** Returns an EPlayerColors::Type indicating which player is the current player */
	EPlayerColors::Type getCurrentPlayer() const;
	/** Returns an EPlayerColors::Type indicating what (if anything) is occupying a given BoardLocation */
	EPlayerColors::Type getOccupier(BoardLocation location) const;
	/** Given a player's color, returns the color of the opponent */
	EPlayerColors::Type getOpponentColor(EPlayerColors::Type color) const;
	/** Returns a reference to the player object corresponding to the given player color. Returns the white player if invalid color is given */
	Player& getPlayer(EPlayerColors::Type playerColor);
	/** Returns the color of the player that won the game. Returns EPlayerColors::Type::NOTHING if the game didn't end yet */
	EPlayerColors::Type getWinner() const;

	/** Resets the game state to the starting setup */
	void reset();

	/** Changes who the current player is */
	void switchCurrentPlayer();

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