#include "GameBoardButton.h"
#include "SerPrunesALotWindow.h"

GameBoardButton::GameBoardButton(int row, int column, QWidget* parent)
	: QPushButton(parent), row(row), column(column)
{
	setAutoDefault(false);

	connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
}

GameBoardButton::~GameBoardButton()
{}

void GameBoardButton::onClicked()
{
	SerPrunesALotWindow* window = (SerPrunesALotWindow*) parent();
	window->buttonClicked(this);
}