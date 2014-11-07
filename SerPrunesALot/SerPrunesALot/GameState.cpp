#include "GameState.h"

GameState::GameState()
	: board(), 
	blackPlayer(EPlayerColors::BLACK_PLAYER), 
	whitePlayer(EPlayerColors::WHITE_PLAYER), 
	currentPlayer(EPlayerColors::WHITE_PLAYER)
{
	// initialize board as empty
	board.reserve(BOARD_HEIGHT);

	for (int i = 0; i < BOARD_HEIGHT; ++i)
	{
		std::vector<EPlayerColors::Type> row;
		row.reserve(BOARD_WIDTH);

		for (int j = 0; j < BOARD_WIDTH; ++j)
		{
			row.push_back(EPlayerColors::NOTHING);
		}

		board.push_back(row);
	}
}

GameState::~GameState()
{}

EPlayerColors::Type GameState::getOccupier(BoardLocation location) const
{
	return board[location.y][location.x];
}

void GameState::reset()
{
	// make sure both players have no Knights from any previous games
	blackPlayer.removeAllKnights();
	whitePlayer.removeAllKnights();

	// fill top 2 rows with black pieces and bottom 2 rows with white pieces
	for (int i = 0; i < BOARD_WIDTH; ++i)
	{
		board[0][i] = EPlayerColors::BLACK_PLAYER;
		board[1][i] = EPlayerColors::BLACK_PLAYER;
		blackPlayer.addKnight(BoardLocation(0, i));
		blackPlayer.addKnight(BoardLocation(1, i));

		board[BOARD_HEIGHT - 1][i] = EPlayerColors::WHITE_PLAYER;
		board[BOARD_HEIGHT - 2][i] = EPlayerColors::WHITE_PLAYER;
		whitePlayer.addKnight(BoardLocation(BOARD_HEIGHT - 1, i));
		whitePlayer.addKnight(BoardLocation(BOARD_HEIGHT - 2, i));
	}

	// empty the middle section
	for (int i = 2; i < BOARD_HEIGHT - 2; ++i)
	{
		for (int j = 0; j < BOARD_WIDTH; ++j)
		{
			board[i][j] = EPlayerColors::NOTHING;
		}
	}

	// reset current player status
	currentPlayer = EPlayerColors::WHITE_PLAYER;
}