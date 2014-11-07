#include "GameConstants.h"
#include "Player.h"
#include "VectorUtils.h"

Player::Player(EPlayerColors::Type color)
	: knightLocations(), color(color)
{
	knightLocations.reserve(BOARD_WIDTH * 2);
}

Player::~Player()
{}

void Player::addKnight(const BoardLocation& boardLoc)
{
	knightLocations.push_back(boardLoc);
}

void Player::removeAllKnights()
{
	knightLocations.clear();
}

void Player::removeKnight(const BoardLocation& boardLoc)
{
	VectorUtils::removeSwap<BoardLocation>(knightLocations, boardLoc);
}