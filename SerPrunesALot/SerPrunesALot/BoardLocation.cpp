#include "BoardLocation.h"
#include "GameConstants.h"

BoardLocation::BoardLocation(int x, int y)
	: x(x), y(y)
{}

bool BoardLocation::isValid() const
{
	return ((BOARD_WIDTH > ((unsigned int)x)) && (BOARD_HEIGHT > ((unsigned int)y)));
}