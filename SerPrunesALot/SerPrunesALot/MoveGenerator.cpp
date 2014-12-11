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
{}

Move MoveGenerator::nextMove()
{
	if(moves.empty() && !generatedMoves)
	{
		moves.reserve(16 * 4);		// upper bound on number of possible moves

		// Add move from TT and any Killer Moves
		if(!(transpositionMove == INVALID_MOVE))
		{
			moves.push_back(transpositionMove);
		}

		// TODO check if Killer moves are valid

		if(!(killerMove1 == INVALID_MOVE))
		{
			moves.push_back(killerMove1);
		}

		if(!(killerMove2 == INVALID_MOVE))
		{
			moves.push_back(killerMove2);
		}
	}

	if(moveIndex >= moves.size() && !generatedMoves)	// already returned TT / Killer Moves, time to generate other moves
	{
		generatedMoves = true;
		moves.clear();	// no longer need TT Move and Killer Moves in our vector

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
						moves.push_back(Move(knightSquare, moveTarget, true));
					}
					else
					{
						nonCaptureMoves.push_back(Move(knightSquare, moveTarget, false));
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