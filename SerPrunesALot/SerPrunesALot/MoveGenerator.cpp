#include "MoveGenerator.h"

#include "GameState.h"

#include <algorithm>

MoveGenerator::MoveGenerator(EPlayerColors::Type playerColor, uint64_t playerBitboard, uint64_t opponentBitboard,
														Move transpositionMove, Move killerMove1, Move killerMove2)
	: moves(),
	transpositionMove(transpositionMove),
	killerMove1(killerMove1),
	killerMove2(killerMove2),
	playerBitboard(playerBitboard),
	opponentBitboard(opponentBitboard),
	playerColor(playerColor),
	moveIndex(0),
	generatedMoves(false)
{
	moves.reserve(16 * 4);		// upper bound on number of possible moves

	// Add move from TT and any Killer Moves
	if(!(transpositionMove == INVALID_MOVE))
	{
		moves.push_back(transpositionMove);
	}
	
	if(!(killerMove1 == INVALID_MOVE) && !(killerMove1 == transpositionMove))
	{
		// check if killer move is actually valid
		uint64_t fromBit = Bitboards::singleBit(killerMove1.from);
		uint64_t toBit = Bitboards::singleBit(killerMove1.to);

		if(fromBit & playerBitboard)		// we have a piece on the from location
		{
			if(!(toBit & playerBitboard))	// we don't have a piece on the to location
			{
				if((killerMove1.captured) == ((opponentBitboard & toBit) != Bitboards::ALL_ZERO))	// capture move iff opponent has piece on to location
				{
					moves.push_back(killerMove1);
				}
			}
		}
	}

	if(!(killerMove2 == INVALID_MOVE) && !(killerMove2 == killerMove1) && !(killerMove2 == transpositionMove))
	{
		// check if killer move is actually valid
		uint64_t fromBit = Bitboards::singleBit(killerMove2.from);
		uint64_t toBit = Bitboards::singleBit(killerMove2.to);

		if(fromBit & playerBitboard)		// we have a piece on the from location
		{
			if(!(toBit & playerBitboard))	// we don't have a piece on the to location
			{
				if((killerMove2.captured) == ((opponentBitboard & toBit) != Bitboards::ALL_ZERO))	// capture move iff opponent has piece on to location
				{
					moves.push_back(killerMove2);
				}
			}
		}
	}
}

Move MoveGenerator::nextMove()
{
	if(moveIndex >= moves.size() && !generatedMoves)	// already returned TT / Killer Moves, time to generate other moves
	{
		generatedMoves = true;
		moves.clear();	// no longer need TT Move and Killer Moves in our vector
		moveIndex = 0;

		// non-capture moves first stored separately
		// this way, capture moves can be ordered before non-capture moves
		std::vector<Move> nonCaptureMoves;
		nonCaptureMoves.reserve(16 * 4);

		uint64_t copyPlayerBitboard = playerBitboard;

		// start scanning our bitboard for 1's
		while(copyPlayerBitboard)
		{
			int knightSquare = Bitboards::bitScanForward(copyPlayerBitboard);
			const std::vector<int>& moveTargets = GameState::getMoveTargets(knightSquare, playerColor);

			for(int moveTarget : moveTargets)
			{
				uint64_t moveTargetBit = Bitboards::singleBit(moveTarget);

				if(!(moveTargetBit & playerBitboard))		// ensure we have no piece on the move target
				{
					if(moveTargetBit & opponentBitboard)	// opponent occupies target location
					{
						Move move(knightSquare, moveTarget, true);

						if(!(move == transpositionMove) && !(move == killerMove1) && !(move == killerMove2))
						{
							moves.push_back(move);
						}
					}
					else
					{
						Move move(knightSquare, moveTarget, false);

						if(!(move == transpositionMove) && !(move == killerMove1) && !(move == killerMove2))
						{
							nonCaptureMoves.push_back(move);
						}
					}
				}
			}

			copyPlayerBitboard &= copyPlayerBitboard - 1;	// set the bit we just processed to 0
		}

		if(playerColor == EPlayerColors::Type::BLACK_PLAYER)	// actually want moves to be in the reverse order for Black
		{
			for(Move m : moves)									// so add the capture moves to the back of non-capture moves
			{
				nonCaptureMoves.push_back(m);
			}

			// now swap contents of ''nonCaptureMoves'' and ''moves'' so that ''moves'' contains all the moves
			moves.swap(nonCaptureMoves);

			// finally, reverse the order
			std::reverse(moves.begin(), moves.end());
		}
		else		// for White, moves are already generated in the preferred order. Just need to merge capture and non-capture moves
		{
			for(Move m : nonCaptureMoves)
			{
				moves.push_back(m);
			}
		}
	}

	Move move = INVALID_MOVE;

	if(moveIndex < moves.size())
	{
		move = moves[moveIndex];
		++moveIndex;
	}

	return move;
}