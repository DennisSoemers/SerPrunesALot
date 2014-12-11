#include "Bitboards.hpp"
#include "BoardUtils.hpp"
#include "GameState.h"
#include "Logger.h"
#include "RNG.h"

std::vector<std::vector<uint64_t>> GameState::zobristRandomNums = std::vector<std::vector<uint64_t>>();
std::vector<std::vector<int>> GameState::moveTargetsBlack = GameState::precomputeMoveTargetsBlack();
std::vector<std::vector<int>> GameState::moveTargetsWhite = GameState::precomputeMoveTargetsWhite();

GameState::GameState()
	: blackBitboard(0),
	whiteBitboard(0),
	zobristHash(0),
	currentPlayer(EPlayerColors::Type::WHITE_PLAYER),
	numBlackKnights(0),
	numWhiteKnights(0)
{
	// initialize random numbers for Zobrist hashing if not done yet
	if (zobristRandomNums.size() == 0)
	{
		zobristRandomNums.reserve(BOARD_HEIGHT * BOARD_WIDTH);

		for (int y = 0; y < BOARD_HEIGHT; ++y)
		{
			for (int x = 0; x < BOARD_WIDTH; ++x)
			{
				// add a vector corresponding to this board location
				std::vector<uint64_t> v;
				v.reserve(NUM_PLAYERS);

				for (int i = 0; i < NUM_PLAYERS; ++i)
				{
					// add a random number corresponding to v's board location and the i'th player
					v.push_back(RNG::randomUint_64());
				}

				zobristRandomNums.push_back(v);
			}
		}
	}

	zobristPlayerNum = RNG::randomUint_64();

#ifdef ALLOW_LOGGING
	if(moveTargetsBlack.empty() || moveTargetsWhite.empty())
	{
		LOG_ERROR("ERROR: GameState::moveTargetsBlack and/or GameState::moveTargetsWhite not initialized!")
	}
#endif // ALLOW_LOGGING
}

GameState::~GameState()
{}

void GameState::applyMove(const Move& move)
{
	// remove opponent piece if we're capturing something
	if (move.captured)
	{
		EPlayerColors::Type opponentColor = getOpponentColor(currentPlayer);

		// account for removal of enemy piece in the zobrist hash value
		zobristHash ^= zobristRandomNums[move.to][opponentColor - 1];

		// update opponent's bitboard
		if(opponentColor == EPlayerColors::Type::BLACK_PLAYER)
		{
			blackBitboard ^= Bitboards::singleBit(move.to);
			--numBlackKnights;
		}
		else
		{
			whiteBitboard ^= Bitboards::singleBit(move.to);
			--numWhiteKnights;
		}
	}

	// update our bitboard
	if(currentPlayer == EPlayerColors::Type::BLACK_PLAYER)
	{
		blackBitboard ^= (Bitboards::singleBit(move.from) ^ Bitboards::singleBit(move.to));
	}
	else
	{
		whiteBitboard ^= (Bitboards::singleBit(move.from) ^ Bitboards::singleBit(move.to));
	}

	// account for movement of our own piece in the zobrist hash value
	zobristHash ^= zobristRandomNums[move.to][currentPlayer - 1];
	zobristHash ^= zobristRandomNums[move.from][currentPlayer - 1];

	// finally, switch player
	switchCurrentPlayer();
}

bool GameState::canMove(int from, int to) const
{
	EPlayerColors::Type player = getOccupier(from);
	return canMove(from, to, player);
}

bool GameState::canMove(int from, int to, EPlayerColors::Type player) const
{
	if (player == getOccupier(to))		// cannot move to square occupied by our own knights
	{
		return false;
	}

	int dx = BoardUtils::x(to) - BoardUtils::x(from);
	int dy = BoardUtils::y(to) - BoardUtils::y(from);

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

std::vector<Move> GameState::generateMoves(int from) const
{
	std::vector<Move> moves;
	moves.reserve(4);	// at most 4 moves from any location. Some cases there will be even fewer moves, but not gonna bother saving that memory

	EPlayerColors::Type player = getOccupier(from);
	const std::vector<int>& moveTargets = GameState::getMoveTargets(from, player);

	uint64_t playerBitboard = (player == EPlayerColors::Type::BLACK_PLAYER) ? blackBitboard : whiteBitboard;
	uint64_t opponentBitboard = (player == EPlayerColors::Type::BLACK_PLAYER) ? whiteBitboard : blackBitboard;

	for(int moveTarget : moveTargets)
	{
		uint64_t moveTargetBit = Bitboards::singleBit(moveTarget);

		if(!(moveTargetBit & playerBitboard))		// ensure we have no piece on the move target
		{
			if(moveTargetBit & opponentBitboard)	// opponent occupies target location
			{
				moves.push_back(Move(from, moveTarget, true));
			}
			else
			{
				moves.push_back(Move(from, moveTarget, false));
			}
		}
	}

	return moves;
}

uint64_t GameState::getBitboard(EPlayerColors::Type player) const
{
	return (player == EPlayerColors::Type::BLACK_PLAYER) ? blackBitboard : whiteBitboard;
}

EPlayerColors::Type GameState::getCurrentPlayer() const
{
	return currentPlayer;
}

/*int GameState::getNumAttackers(const BoardLocation& location, EPlayerColors::Type attackersColor) const
{
	int numAttackers = 0;

	if (attackersColor == EPlayerColors::Type::BLACK_PLAYER)
	{
		// generate the 4 potential attacker locations hardcoded because we like speed
		BoardLocation vertVertLeft(location.x - 1, location.y - 2);
		BoardLocation vertVertRight(location.x + 1, location.y - 2);
		BoardLocation vertLeftLeft(location.x - 2, location.y - 1);
		BoardLocation vertRightRight(location.x + 2, location.y - 1);

		// test for each potential attacker location if it's actually on the board and occupied by an attacker
		if (vertVertLeft.isValid() && getOccupier(vertVertLeft) == attackersColor)
		{
			++numAttackers;
		}
		if (vertVertRight.isValid() && getOccupier(vertVertRight) == attackersColor)
		{
			++numAttackers;
		}
		if (vertLeftLeft.isValid() && getOccupier(vertLeftLeft) == attackersColor)
		{
			++numAttackers;
		}
		if (vertRightRight.isValid() && getOccupier(vertRightRight) == attackersColor)
		{
			++numAttackers;
		}
	}
	else 
	{
		// generate the 4 potential attacker locations hardcoded because we like speed
		BoardLocation vertLeftLeft(location.x - 2, location.y + 1);
		BoardLocation vertRightRight(location.x + 2, location.y + 1);
		BoardLocation vertVertLeft(location.x - 1, location.y + 2);
		BoardLocation vertVertRight(location.x + 1, location.y + 2);

		// test for each potential attacker location if it's actually on the board and occupied by an attacker
		if (vertLeftLeft.isValid() && getOccupier(vertLeftLeft) == attackersColor)
		{
			++numAttackers;
		}
		if (vertRightRight.isValid() && getOccupier(vertRightRight) == attackersColor)
		{
			++numAttackers;
		}
		if (vertVertLeft.isValid() && getOccupier(vertVertLeft) == attackersColor)
		{
			++numAttackers;
		}
		if (vertVertRight.isValid() && getOccupier(vertVertRight) == attackersColor)
		{
			++numAttackers;
		}
	}

	return numAttackers;
}*/

const std::vector<int>& GameState::getMoveTargets(int location, EPlayerColors::Type color)
{
	return (color == EPlayerColors::Type::BLACK_PLAYER) ? moveTargetsBlack[location] : moveTargetsWhite[location];
}

EPlayerColors::Type GameState::getOccupier(int location) const
{
	uint64_t locationBit = Bitboards::singleBit(location);

	if(blackBitboard & locationBit)
	{
		return EPlayerColors::Type::BLACK_PLAYER;
	}
	else if(whiteBitboard & locationBit)
	{
		return EPlayerColors::Type::WHITE_PLAYER;
	}
	else
	{
		return EPlayerColors::Type::NOTHING;
	}
}

int GameState::getNumBlackKnights() const
{
	return numBlackKnights;
}

int GameState::getNumWhiteKnights() const
{
	return numWhiteKnights;
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

EPlayerColors::Type GameState::getWinner() const
{
	if(blackBitboard & Bitboards::ROW_1)		// non-zero intersection between black bitboard and bottom row
	{
		return EPlayerColors::Type::BLACK_PLAYER;
	}
	else if(whiteBitboard & Bitboards::ROW_8)	// non-zero intersection between white bitboard and top row
	{
		return EPlayerColors::Type::WHITE_PLAYER;
	}

	if (numBlackKnights == 0)
	{
		return EPlayerColors::Type::WHITE_PLAYER;
	}
	else if (numWhiteKnights == 0)
	{
		return EPlayerColors::Type::BLACK_PLAYER;
	}

	return EPlayerColors::Type::NOTHING;
}

uint64_t GameState::getZobrist() const
{
	return zobristHash;
}

bool GameState::isMoveLegal(const Move& move) const
{
	if (currentPlayer == getOccupier(move.to))		// cannot move to square occupied by our own knights
	{
		return false;
	}

	if (currentPlayer != getOccupier(move.from))	// cannot move from a location we do not occupy
	{
		return false;
	}

	if (move.captured != (getOpponentColor(currentPlayer) == getOccupier(move.to)))		// must capture if enemy occupies, and cannot capture if he doesn't
	{
		return false;
	}

	int dx = BoardUtils::x(move.to) - BoardUtils::x(move.from);
	int dy = BoardUtils::y(move.to) - BoardUtils::y(move.from);

	if (currentPlayer == EPlayerColors::Type::BLACK_PLAYER)		// must move down
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
	else if (currentPlayer == EPlayerColors::Type::WHITE_PLAYER)	// must move up
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

void GameState::reset()
{
	// reset number of knights for each player
	numBlackKnights = 16;
	numWhiteKnights = 16;

	// set current zobrist hash value to the zobrist player number, to indicate it's white player's turn
	zobristHash = zobristPlayerNum;

	// fill top 2 rows with black pieces and bottom 2 rows with white pieces
	blackBitboard = Bitboards::ROW_8 | Bitboards::ROW_7;
	whiteBitboard = Bitboards::ROW_1 | Bitboards::ROW_2;

	// update zobrist hash value
	for (int i = 0; i < BOARD_WIDTH; ++i)
	{
		zobristHash ^= zobristRandomNums[0 * BOARD_HEIGHT + i][EPlayerColors::Type::BLACK_PLAYER - 1];
		zobristHash ^= zobristRandomNums[1 * BOARD_HEIGHT + i][EPlayerColors::Type::BLACK_PLAYER - 1];
		zobristHash ^= zobristRandomNums[(BOARD_HEIGHT - 1) * BOARD_HEIGHT + i][EPlayerColors::Type::WHITE_PLAYER - 1];
		zobristHash ^= zobristRandomNums[(BOARD_HEIGHT - 2) * BOARD_HEIGHT + i][EPlayerColors::Type::WHITE_PLAYER - 1];
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

	// update zobrist hash
	zobristHash ^= zobristPlayerNum;
}

void GameState::undoMove(const Move& move)
{
	// switch current player back
	switchCurrentPlayer();

	// give opponent piece back if we captured something
	if (move.captured)
	{
		EPlayerColors::Type opponentColor = getOpponentColor(currentPlayer);

		// account for removal of enemy piece in the zobrist hash value
		zobristHash ^= zobristRandomNums[move.to][opponentColor - 1];

		// update opponent's bitboard
		if(opponentColor == EPlayerColors::Type::BLACK_PLAYER)
		{
			blackBitboard ^= Bitboards::singleBit(move.to);
			++numBlackKnights;
		}
		else
		{
			whiteBitboard ^= Bitboards::singleBit(move.to);
			++numWhiteKnights;
		}
	}

	// update our bitboard
	if(currentPlayer == EPlayerColors::Type::BLACK_PLAYER)
	{
		blackBitboard ^= (Bitboards::singleBit(move.from) ^ Bitboards::singleBit(move.to));
	}
	else
	{
		whiteBitboard ^= (Bitboards::singleBit(move.from) ^ Bitboards::singleBit(move.to));
	}

	// account for movement of our own piece in the zobrist hash value
	zobristHash ^= zobristRandomNums[move.to][currentPlayer - 1];
	zobristHash ^= zobristRandomNums[move.from][currentPlayer - 1];
}

std::vector<std::vector<int>> GameState::precomputeMoveTargetsBlack()
{
	std::vector<std::vector<int>> moves;

	for(int y = 0; y < BOARD_HEIGHT; ++y)
	{
		for(int x = 0; x < BOARD_WIDTH; ++x)
		{
			int boardLocation = BoardUtils::coordsToIndex(x, y);

			std::vector<int> moveTargets;
			moveTargets.reserve(4);

			if(y < BOARD_HEIGHT - 1)		// can move at least one square down
			{
				if(x < BOARD_WIDTH - 2)			// can move at least two squares to the right
				{
					moveTargets.push_back(BoardUtils::coordsToIndex(x + 2, y + 1));
				}

				if(x > 2)						// can move at least two squares to the left
				{
					moveTargets.push_back(BoardUtils::coordsToIndex(x - 2, y + 1));
				}

				if(y < BOARD_HEIGHT - 2)		// can move at least two squares down
				{
					if(x < BOARD_WIDTH - 1)			// can move at least one square to the right
					{
						moveTargets.push_back(BoardUtils::coordsToIndex(x + 1, y + 2));
					}

					if(x > 1)						// can move at least one square to the left
					{
						moveTargets.push_back(BoardUtils::coordsToIndex(x - 1, y + 2));
					}
				}
			}

			moveTargetsBlack.push_back(moveTargets);
		}
	}

	return moves;
}

std::vector<std::vector<int>> GameState::precomputeMoveTargetsWhite()
{
	std::vector<std::vector<int>> moves;

	for(int y = 0; y < BOARD_HEIGHT; ++y)
	{
		for(int x = 0; x < BOARD_WIDTH; ++x)
		{
			int boardLocation = BoardUtils::coordsToIndex(x, y);

			std::vector<int> moveTargets;
			moveTargets.reserve(4);

			if(y > 0)						// can move at least one square up
			{
				if(x < BOARD_WIDTH - 2)			// can move at least two squares to the right
				{
					moveTargets.push_back(BoardUtils::coordsToIndex(x + 2, y - 1));
				}

				if(x > 2)						// can move at least two squares to the left
				{
					moveTargets.push_back(BoardUtils::coordsToIndex(x - 2, y - 1));
				}

				if(y > 1)						// can move at least two squares up
				{
					if(x < BOARD_WIDTH - 1)			// can move at least one square to the right
					{
						moveTargets.push_back(BoardUtils::coordsToIndex(x + 1, y - 2));
					}

					if(x > 1)						// can move at least one square to the left
					{
						moveTargets.push_back(BoardUtils::coordsToIndex(x - 1, y - 2));
					}
				}
			}

			moveTargetsWhite.push_back(moveTargets);
		}
	}

	return moves;
}