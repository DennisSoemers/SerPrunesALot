#include "Move.h"

Move::Move(BoardLocation from, BoardLocation to, bool captured)
	: from(from), to(to), captured(captured)
{}