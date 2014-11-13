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

void Player::addKnight(BoardLocation boardLoc)
{
	knightLocations.push_back(boardLoc);
}

std::vector<BoardLocation>& Player::getKnightLocations()
{
	return knightLocations;
}

const std::vector<BoardLocation>& Player::getKnightLocations() const
{
	return knightLocations;
}

int Player::getNumKnights() const
{
	return knightLocations.size();
}

void Player::removeAllKnights()
{
	knightLocations.clear();
}

void Player::removeKnight(BoardLocation boardLoc)
{
	VectorUtils::removeSwap<BoardLocation>(knightLocations, boardLoc);
}