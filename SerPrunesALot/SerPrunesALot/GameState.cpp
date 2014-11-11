#include "GameState.h"

GameState::GameState()
	: board(BOARD_HEIGHT, std::vector<EPlayerColors::Type>(BOARD_WIDTH)), 
	blackPlayer(EPlayerColors::Type::BLACK_PLAYER), 
	whitePlayer(EPlayerColors::Type::WHITE_PLAYER), 
	currentPlayer(EPlayerColors::Type::WHITE_PLAYER)
{
	// initialize board as empty
	board.reserve(BOARD_HEIGHT);

	for (int i = 0; i < BOARD_HEIGHT; ++i)
	{
		for (int j = 0; j < BOARD_WIDTH; ++j)
		{
			board[i][j] = EPlayerColors::Type::NOTHING;
		}
	}
}

GameState::~GameState()
{}

void GameState::applyMove(const Move& move)
{
	// remove opponent piece if we're capturing something
	if (move.captured)
	{
		getPlayer(getOpponentColor(currentPlayer)).removeKnight(move.to);
	}

	// change the board data
	board[move.from.y][move.from.x] = EPlayerColors::Type::NOTHING;
	board[move.to.y][move.to.x] = currentPlayer;

	// find the ''from'' location in our list of knight locations, and update it to the ''to'' location
	std::vector<BoardLocation> knightLocations = getPlayer(currentPlayer).getKnightLocations();

	for (size_t i = 0; i < knightLocations.size(); ++i)
	{
		BoardLocation& loc = knightLocations[i];

		if (loc == move.from)
		{
			loc.x = move.to.x;
			loc.y = move.to.y;
			return;
		}
	}

	// TODO: shouldn't be possible to reach this, log some kind of error
}

bool GameState::canMove(BoardLocation from, BoardLocation to) const
{
	EPlayerColors::Type player = getOccupier(from);
	return canMove(from, to, player);
}

bool GameState::canMove(BoardLocation from, BoardLocation to, EPlayerColors::Type player) const
{
	if (player == getOccupier(to))		// cannot move to square occupied by our own knights
	{
		return false;
	}

	int dx = to.x - from.x;
	int dy = to.y - from.y;

	if (player == EPlayerColors::Type::BLACK_PLAYER)		// must move down
	{
		if (dy == 1)
		{
			return (dx == -2) || (dx == 2);
		}
		else if (dy == 2)
		{
			return (dx == -1) || (dx == 1);
		}
	}
	else if (player == EPlayerColors::Type::WHITE_PLAYER)	// must move up
	{
		if (dy == -1)
		{
			return (dx == -2) || (dx == 2);
		}
		else if (dy == -2)
		{
			return (dx == -1) || (dx == 1);
		}
	}

	return false;
}

std::vector<Move> GameState::generateMoves(BoardLocation from) const
{
	std::vector<Move> moves;
	moves.reserve(4);	// at most 4 moves from any location. Some cases there will be even fewer moves, but not gonna bother saving that memory

	EPlayerColors::Type player = getOccupier(from);
	int dy = (player == EPlayerColors::Type::BLACK_PLAYER) ? 1 : -1;	// move down if black, or up if white

	// generate the 4 potential moves hardcoded because we like speed
	BoardLocation vertLeftLeft(from.x - 2, from.y + dy * 1);
	BoardLocation vertRightRight(from.x + 2, from.y + dy * 1);
	BoardLocation vertVertLeft(from.x - 1, from.y + dy * 2);
	BoardLocation vertVertRight(from.x + 1, from.y + dy * 2);

	// test for each potential move if it's actually on the board and not occupied by our own knights
	if (vertLeftLeft.isValid() && getOccupier(vertLeftLeft) != player)
	{
		moves.push_back(Move(from, vertLeftLeft, (getOccupier(vertLeftLeft) != EPlayerColors::Type::NOTHING)));
	}
	if (vertRightRight.isValid() && getOccupier(vertRightRight) != player)
	{
		moves.push_back(Move(from, vertRightRight, (getOccupier(vertRightRight) != EPlayerColors::Type::NOTHING)));
	}
	if (vertVertLeft.isValid() && getOccupier(vertVertLeft) != player)
	{
		moves.push_back(Move(from, vertVertLeft, (getOccupier(vertVertLeft) != EPlayerColors::Type::NOTHING)));
	}
	if (vertVertRight.isValid() && getOccupier(vertVertRight) != player)
	{
		moves.push_back(Move(from, vertVertRight, (getOccupier(vertVertRight) != EPlayerColors::Type::NOTHING)));
	}

	return moves;
}

EPlayerColors::Type GameState::getCurrentPlayer() const
{
	return currentPlayer;
}

EPlayerColors::Type GameState::getOccupier(BoardLocation location) const
{
	return board[location.y][location.x];
}

EPlayerColors::Type GameState::getOpponentColor(EPlayerColors::Type color) const
{
	if (color == EPlayerColors::Type::BLACK_PLAYER)
	{
		return EPlayerColors::Type::WHITE_PLAYER;
	}
	else if (color == EPlayerColors::Type::WHITE_PLAYER)
	{
		return EPlayerColors::Type::BLACK_PLAYER;
	}
	else
	{
		return EPlayerColors::Type::NOTHING;
	}
}

Player& GameState::getPlayer(EPlayerColors::Type playerColor)
{
	if (playerColor == EPlayerColors::Type::BLACK_PLAYER)
	{
		return blackPlayer;
	}
	else
	{
		return whitePlayer;
	}
}

void GameState::reset()
{
	// make sure both players have no Knights from any previous games
	blackPlayer.removeAllKnights();
	whitePlayer.removeAllKnights();

	// fill top 2 rows with black pieces and bottom 2 rows with white pieces
	for (int i = 0; i < BOARD_WIDTH; ++i)
	{
		board[0][i] = EPlayerColors::Type::BLACK_PLAYER;
		board[1][i] = EPlayerColors::Type::BLACK_PLAYER;
		blackPlayer.addKnight(BoardLocation(0, i));
		blackPlayer.addKnight(BoardLocation(1, i));

		board[BOARD_HEIGHT - 1][i] = EPlayerColors::Type::WHITE_PLAYER;
		board[BOARD_HEIGHT - 2][i] = EPlayerColors::Type::WHITE_PLAYER;
		whitePlayer.addKnight(BoardLocation(BOARD_HEIGHT - 1, i));
		whitePlayer.addKnight(BoardLocation(BOARD_HEIGHT - 2, i));
	}

	// empty the middle section
	for (int i = 2; i < BOARD_HEIGHT - 2; ++i)
	{
		for (int j = 0; j < BOARD_WIDTH; ++j)
		{
			board[i][j] = EPlayerColors::Type::NOTHING;
		}
	}

	// reset current player status
	currentPlayer = EPlayerColors::Type::WHITE_PLAYER;
}

void GameState::switchCurrentPlayer()
{
	if (currentPlayer == EPlayerColors::Type::WHITE_PLAYER)
	{
		currentPlayer = EPlayerColors::Type::BLACK_PLAYER;
	}
	else
	{
		currentPlayer = EPlayerColors::Type::WHITE_PLAYER;
	}
}