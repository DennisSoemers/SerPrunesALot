#pragma once

#include "QPushButton.h"
#include "QWidget.h"

/**
 * Class for the Game Board Buttons (= the buttons on the game board that can be clicked to command
 * Knights to move).
 */
class GameBoardButton : public QPushButton
{
public:
	GameBoardButton(QWidget* parent = nullptr);
	~GameBoardButton();

private:
	Q_DISABLE_COPY(GameBoardButton)
};