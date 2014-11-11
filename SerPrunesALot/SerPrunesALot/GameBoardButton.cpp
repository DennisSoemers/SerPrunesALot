#include "GameBoardButton.h"
#include "SerPrunesALotWindow.h"

GameBoardButton::GameBoardButton(int row, int column, QWidget* parent)
	: QPushButton(parent), row(row), column(column)
{
	setAutoDefault(false);
}

GameBoardButton::~GameBoardButton()
{}

void GameBoardButton::onClicked()
{
	SerPrunesALotWindow* window = dynamic_cast<SerPrunesALotWindow*>(parent()->parent());

	if (window)
	{
		window->buttonClicked(this);
	}
}

void GameBoardButton::resetBackgroundColor()
{
	if ((row % 2) == (column % 2))
	{
		setStyleSheet("background-color:white;");
	}
	else
	{
		setStyleSheet("background-color:brown;");
	}
}