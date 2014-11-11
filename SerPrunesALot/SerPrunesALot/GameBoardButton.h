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
	GameBoardButton(int row, int column, QWidget* parent = nullptr);
	~GameBoardButton();

private slots:
	void onClicked();

private:
	Q_DISABLE_COPY(GameBoardButton)

	int row;
	int column;
};