#pragma once

#include <QtWidgets/QPushButton>

/**
 * Class for the Game Board Buttons (= the buttons on the game board that can be clicked to command
 * Knights to move).
 */
class GameBoardButton : public QPushButton
{
	Q_OBJECT

public:
	GameBoardButton(int row, int column, QWidget* parent = nullptr);
	~GameBoardButton();

	/** Resets this button's background color */
	void resetBackgroundColor();

	int row;
	int column;

public slots:
	void onClicked();

private:
	Q_DISABLE_COPY(GameBoardButton)
};