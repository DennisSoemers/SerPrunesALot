#include "BasicAlphaBeta.h"
#include "MathConstants.h"

Move BasicAlphaBeta::chooseMove(GameState& gameState)
{
	return startAlphaBeta(gameState, 2);
}

int BasicAlphaBeta::alphaBeta(GameState& gameState, int depth, int alpha, int beta)
{

}

Move BasicAlphaBeta::startAlphaBeta(GameState& gameState, int depth)
{
	std::vector<Move> moves = gameState.generateAllMoves();
	Move bestMove = moves.at(0);
	int bestScore = MathConstants::MIN_INT;

	// stop search if we reached max depth or have found a winner
	if (depth == 0 || gameState.getWinner() != EPlayerColors::Type::NOTHING)
	{
		return evaluate(gameState);
	}
}