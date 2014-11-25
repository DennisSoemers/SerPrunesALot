#include "SerPrunesALotWindow.h"
#include "QActionGroup.h"
#include "QDesktopWidget.h"
#include "QGridLayout.h"
#include "QIcon.h"
#include "QLabel.h"

#include <cmath>

#include "Options.h"

#include "AlphaBetaTT.h"
#include "BasicAlphaBeta.h"
#include "Logger.h"
#include "Move.h"
#include "TranspositionTable.h"

SerPrunesALotWindow::SerPrunesALotWindow(QWidget *parent)
	: QMainWindow(parent),
	boardButtons(),
	highlightedButtons(),
	currentGameState(),
	selectedButton(nullptr),
	ui(),
	aiEngineBlack(nullptr),
	aiEngineWhite(nullptr)
{
	// NOTE: hardcoding this means only board sizes up to 8x8 are supported
	char* COORDS_NUMBERS[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
	char* COORDS_LETTERS[] = { "A", "B", "C", "D", "E", "F", "G", "H" };

	ui.setupUi(this);	// call the setup method that was automatically generated by the project creation wizard

	// other setup stuff
	setWindowTitle("Ser Prunes-A-Lot");
	setCentralWidget(new QWidget(this));

	// Place window in middle of screen
	QDesktopWidget* desktop = QApplication::desktop();

	int screenWidth = desktop->width();
	int screenHeight = desktop->height();

	int windowWidth = 800;
	int windowHeight = 750;

	resize(windowWidth, windowHeight);
	move(((screenWidth - windowWidth) / 2), ((screenHeight - windowHeight) / 2));

	// Set window icon
	setWindowIcon(QIcon(":/Resources/Resources/WhiteIcon.png"));

	// Prepare layout for gameboard
	QGridLayout* gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(0);

	// draw letters and numbers as ''coordinates'' for buttons
	for (int i = 1; i <= BOARD_HEIGHT; ++i)
	{
		QLabel* labelLeft = new QLabel(QString::fromLatin1(COORDS_NUMBERS[BOARD_HEIGHT - i]), this);
		labelLeft->setMinimumWidth(30);
		gridLayout->addWidget(labelLeft, i, 0);

		QLabel* labelRight = new QLabel(QString("     ").append(QString::fromLatin1(COORDS_NUMBERS[BOARD_HEIGHT - i])), this);
		labelRight->setMinimumWidth(30);
		gridLayout->addWidget(labelRight, i, BOARD_WIDTH + 1);
	}

	for (int i = 0; i < BOARD_WIDTH; ++i)
	{
		QLabel* labelTop = new QLabel(QString("     ").append(QString::fromLatin1(COORDS_LETTERS[i])), this);
		labelTop->setMinimumHeight(30);
		gridLayout->addWidget(labelTop, 0, i + 1);

		QLabel* labelBot = new QLabel(QString("       ").append(QString::fromLatin1(COORDS_LETTERS[i])), this);
		labelBot->setMinimumHeight(30);
		gridLayout->addWidget(labelBot, BOARD_HEIGHT + 1, i + 1);
	}

	// fill board with buttons
	boardButtons.reserve(BOARD_HEIGHT);

	for (int i = 0; i < BOARD_HEIGHT; ++i)
	{
		std::vector<GameBoardButton*> row;
		row.reserve(BOARD_WIDTH);

		for (int j = 0; j < BOARD_WIDTH; ++j)
		{
			GameBoardButton* button = new GameBoardButton(i, j, this);

			button->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
			button->setToolTip(QString::fromLatin1(COORDS_LETTERS[j]).append(QString::fromLatin1(COORDS_NUMBERS[BOARD_HEIGHT - i - 1])));
			button->resetBackgroundColor();

			row.push_back(button);
			gridLayout->addWidget(button, i + 1, j + 1);

			connect(button, &GameBoardButton::clicked, button, &GameBoardButton::onClicked);
		}

		boardButtons.push_back(row);
	}

	centralWidget()->setLayout(gridLayout);

	// Create menus for menu bar
	// Player Options
	QMenu* playerMenu = new QMenu("Player Options");
	QMenu* blackPlayerMenu = new QMenu("Black Player");
	QMenu* whitePlayerMenu = new QMenu("White Player");

	blackPlayerAiControl = new QAction("Allow AI control", blackPlayerMenu);
	blackPlayerAiControl->setCheckable(true);
	blackPlayerAiControl->setChecked(true);
	blackPlayerMenu->addAction(blackPlayerAiControl);

	blackPlayerManualControl = new QAction("Allow manual control", blackPlayerMenu);
	blackPlayerManualControl->setCheckable(true);
	blackPlayerManualControl->setChecked(true);
	blackPlayerMenu->addAction(blackPlayerManualControl);

	whitePlayerAiControl = new QAction("Allow AI control", whitePlayerMenu);
	whitePlayerAiControl->setCheckable(true);
	whitePlayerAiControl->setChecked(true);
	whitePlayerMenu->addAction(whitePlayerAiControl);

	whitePlayerManualControl = new QAction("Allow manual control", whitePlayerMenu);
	whitePlayerManualControl->setCheckable(true);
	whitePlayerManualControl->setChecked(true);
	whitePlayerMenu->addAction(whitePlayerManualControl);

	playerMenu->addMenu(blackPlayerMenu);
	playerMenu->addMenu(whitePlayerMenu);

	// Create and add the button to let the AI play a turn
	QAction* runAiButton = new QAction("Run AI this turn!", menuBar());
	connect(runAiButton, &QAction::triggered, this, &SerPrunesALotWindow::playTurnAi);

	// Create and add button to undo the last move
	QAction* undoLastMoveButton = new QAction("Undo Last Move", menuBar());
	connect(undoLastMoveButton, &QAction::triggered, this, &SerPrunesALotWindow::undoLastMove);

	// Create and add button to highlight all possible moves this turn
	QAction* highlightAllMovesButton = new QAction("Highlight All Moves", menuBar());
	connect(highlightAllMovesButton, &QAction::triggered, this, &SerPrunesALotWindow::highlightAllMoves);

	// Create menus to choose AI engines
	QMenu* chooseEngineMenu = new QMenu("Choose AI Engine");
	QMenu* blackEngineMenu = new QMenu("Black Player");
	QMenu* whiteEngineMenu = new QMenu("White Player");

	// Create the options
	QActionGroup* blackEngines = new QActionGroup(blackEngineMenu);
	QActionGroup* whiteEngines = new QActionGroup(whiteEngineMenu);

	blackPlayerBasicAlphaBeta = new QAction("Basic Alpha-Beta", blackEngines);
	blackPlayerAlphaBetaTT = new QAction("Alpha-Beta with Transposition Table", blackEngines);
	whitePlayerBasicAlphaBeta = new QAction("Basic Alpha-Beta", whiteEngines);
	whitePlayerAlphaBetaTT = new QAction("Alpha-Beta with Transposition Table", whiteEngines);

	// Connect buttons to functions
	connect(blackPlayerBasicAlphaBeta, &QAction::triggered, this, &SerPrunesALotWindow::resetBlackAiEngine);
	connect(blackPlayerAlphaBetaTT, &QAction::triggered, this, &SerPrunesALotWindow::resetBlackAiEngine);
	connect(whitePlayerBasicAlphaBeta, &QAction::triggered, this, &SerPrunesALotWindow::resetWhiteAiEngine);
	connect(whitePlayerAlphaBetaTT, &QAction::triggered, this, &SerPrunesALotWindow::resetWhiteAiEngine);

	// Add buttons to groups
	blackPlayerBasicAlphaBeta->setActionGroup(blackEngines);
	blackPlayerAlphaBetaTT->setActionGroup(blackEngines);
	whitePlayerBasicAlphaBeta->setActionGroup(whiteEngines);
	whitePlayerAlphaBetaTT->setActionGroup(whiteEngines);

	// Make the buttons checkable
	blackPlayerBasicAlphaBeta->setCheckable(true);
	blackPlayerAlphaBetaTT->setCheckable(true);
	whitePlayerBasicAlphaBeta->setCheckable(true);
	whitePlayerAlphaBetaTT->setCheckable(true);

	// Set the initially checked buttons
	blackPlayerAlphaBetaTT->setChecked(true);
	whitePlayerAlphaBetaTT->setChecked(true);

	// Add buttons to submenus, and submenus to main menu
	blackEngineMenu->addAction(blackPlayerBasicAlphaBeta);
	blackEngineMenu->addAction(blackPlayerAlphaBetaTT);
	whiteEngineMenu->addAction(whitePlayerBasicAlphaBeta);
	whiteEngineMenu->addAction(whitePlayerAlphaBetaTT);

	chooseEngineMenu->addMenu(blackEngineMenu);
	chooseEngineMenu->addMenu(whiteEngineMenu);

	// fill menuBar
	menuBar()->addMenu(playerMenu);
	menuBar()->addAction(undoLastMoveButton);
	menuBar()->addAction(highlightAllMovesButton);
	menuBar()->addMenu(chooseEngineMenu);
	menuBar()->addAction(runAiButton);

	// set up status bar
	winDetectionLabel = new QLabel();
	statusBar()->addPermanentWidget(winDetectionLabel);

	// create AI Engines
	aiEngineBlack = new AlphaBetaTT();
	aiEngineWhite = new AlphaBetaTT();

	// Initialize the board for new game
	initBoard();

	// log some initial info
	LOG_SIZE_OF_PRIMITIVES()
	LOG_SIZE_OF(Move)
	LOG_SIZE_OF(HashValue)
	LOG_SIZE_OF(TableData)
	LOG_SIZE_OF(TableEntry)
	LOG_MESSAGE(StringBuilder() << "Transposition Table occupies " << sizeof(TableEntry) * TRANSPOSITION_TABLE_NUM_ENTRIES << " bytes (= " 
																	<< sizeof(TableEntry) * TRANSPOSITION_TABLE_NUM_ENTRIES / 1024.0 / 1024.0 << " MB)")
}

SerPrunesALotWindow::~SerPrunesALotWindow()
{
	if (aiEngineBlack)
	{
		delete aiEngineBlack;
		aiEngineBlack = nullptr;
	}

	if (aiEngineWhite)
	{
		delete aiEngineWhite;
		aiEngineWhite = nullptr;
	}
}

void SerPrunesALotWindow::buttonClicked(GameBoardButton* button)
{
	BoardLocation clickedLoc = BoardLocation(button->column, button->row);
	EPlayerColors::Type occupier = currentGameState.getOccupier(clickedLoc);
	EPlayerColors::Type currentPlayer = currentGameState.getCurrentPlayer();

	if (occupier == currentPlayer)		// clicked a button that we have a knight on
	{
		selectedButton = button;

		// revert all currently highlighted buttons back to their normal color
		for (GameBoardButton* highlighted : highlightedButtons)
		{
			highlighted->resetBackgroundColor();
		}
		highlightedButtons.clear();

		// generatete moves possible from this location and highlight them
		std::vector<Move> moves = currentGameState.generateMoves(clickedLoc);
		for (const Move& m : moves)
		{
			GameBoardButton* targetButton = boardButtons[m.to.y][m.to.x];
			targetButton->setStyleSheet("background-color:green;");
			highlightedButtons.push_back(targetButton);
		}
	}
	else if (selectedButton && currentGameState.canMove(BoardLocation(selectedButton->column, selectedButton->row), clickedLoc, currentPlayer))
	{
		// verify that we actually allow manual control
		bool allowManualControl = false;

		if (currentPlayer == EPlayerColors::BLACK_PLAYER && blackPlayerManualControl->isChecked())
		{
			allowManualControl = true;
		}
		else if (currentPlayer == EPlayerColors::WHITE_PLAYER && whitePlayerManualControl->isChecked())
		{
			allowManualControl = true;
		}

		if (allowManualControl)
		{
			// revert all currently highlighted buttons back to their normal color
			for (GameBoardButton* highlighted : highlightedButtons)
			{
				highlighted->resetBackgroundColor();
			}
			highlightedButtons.clear();

			// generate the move we're playing
			bool capture = (occupier == currentGameState.getOpponentColor(currentPlayer));
			Move move(BoardLocation(selectedButton->column, selectedButton->row), clickedLoc, capture);

			// apply the move and update GUI status
			currentGameState.applyMove(move);
			updateGui();
			movesPlayed.push_back(move);

			// highlight the squares we came from and went to
			selectedButton->setStyleSheet("background-color:blue;");
			highlightedButtons.push_back(selectedButton);
			if (move.captured)
			{
				button->setStyleSheet("background-color:red;");
			}
			else
			{
				button->setStyleSheet("background-color:blue;");
			}
			highlightedButtons.push_back(button);

			selectedButton = nullptr;
		}
	}
}

void SerPrunesALotWindow::initBoard()
{
	currentGameState.reset();
	winDetectionLabel->setText("");
}

void SerPrunesALotWindow::highlightAllMoves()
{
	// revert all currently highlighted buttons back to their normal color
	for (GameBoardButton* highlighted : highlightedButtons)
	{
		highlighted->resetBackgroundColor();
	}
	highlightedButtons.clear();

	// generate all moves
	std::vector<Move> moves = currentGameState.generateAllMoves();

	// highlight them all
	for (Move m : moves)
	{
		boardButtons[m.to.y][m.to.x]->setStyleSheet("background-color:blue;");
		highlightedButtons.push_back(boardButtons[m.to.y][m.to.x]);
	}
}

void SerPrunesALotWindow::playTurnAi()
{
	EPlayerColors::Type currentPlayer = currentGameState.getCurrentPlayer();

	AiEngine* aiEngine = (currentPlayer == EPlayerColors::Type::BLACK_PLAYER) ? aiEngineBlack : aiEngineWhite;

	if (!aiEngine)
	{
		return;
	}

	// Make sure we allow AI to control the color player that is allowed to play this turn
	if (currentPlayer == EPlayerColors::Type::BLACK_PLAYER && !blackPlayerAiControl->isChecked())
	{
		return;
	}

	if (currentPlayer == EPlayerColors::Type::WHITE_PLAYER && !whitePlayerAiControl->isChecked())
	{
		return;
	}

	statusBar()->showMessage("Running AI engine...");

	// let our AI Engine choose a move
	Move move = aiEngine->chooseMove(currentGameState);

	if (move == INVALID_MOVE)		// means game is already over
	{
		statusBar()->showMessage("Game ended!");
		return;
	}

	statusBar()->showMessage("Waiting for input...");
	int rootEvaluation = aiEngine->getRootEvaluation();

	if (rootEvaluation == aiEngine->getWinEvaluation())				// detected (future) win
	{
		if (currentPlayer == EPlayerColors::Type::WHITE_PLAYER)
		{
			winDetectionLabel->setText("(Future) Win detected for White Player by White Player!");
		}
		else
		{
			winDetectionLabel->setText("(Future) Win detected for Black Player by White Player!");
		}
	}
	else if (rootEvaluation == -aiEngine->getWinEvaluation())		// detected (future) loss
	{
		if (currentPlayer == EPlayerColors::Type::WHITE_PLAYER)
		{
			winDetectionLabel->setText("(Future) Win detected for Black Player by White Player!");
		}
		else
		{
			winDetectionLabel->setText("(Future) Win detected for White Player by Black Player!");
		}
	}
	else
	{
		if (currentPlayer == EPlayerColors::Type::WHITE_PLAYER)
		{
			winDetectionLabel->setText(QString::fromStdString(StringBuilder() << "White player evaluates this match: " << rootEvaluation));
		}
		else
		{
			winDetectionLabel->setText(QString::fromStdString(StringBuilder() << "Black player evaluates this match: " << rootEvaluation));
		}
	}

	// revert all currently highlighted buttons back to their normal color
	for (GameBoardButton* highlighted : highlightedButtons)
	{
		highlighted->resetBackgroundColor();
	}
	highlightedButtons.clear();

	// apply the move and update GUI status
	currentGameState.applyMove(move);
	updateGui();
	movesPlayed.push_back(move);

	// highlight the squares we came from and went to
	boardButtons[move.from.y][move.from.x]->setStyleSheet("background-color:blue;");
	highlightedButtons.push_back(boardButtons[move.from.y][move.from.x]);
	if (move.captured)
	{
		boardButtons[move.to.y][move.to.x]->setStyleSheet("background-color:red;");
	}
	else
	{
		boardButtons[move.to.y][move.to.x]->setStyleSheet("background-color:blue;");
	}
	highlightedButtons.push_back(boardButtons[move.to.y][move.to.x]);

	selectedButton = nullptr;

	// if game is over, and we want to log stats at end of game, do so
#ifdef LOG_STATS_END_OF_MATCH
	if (currentGameState.getWinner() != EPlayerColors::Type::NOTHING)
	{
		aiEngine->logEndOfMatchStats();
	}
#endif
}

void SerPrunesALotWindow::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);

	updateGui();
}

void SerPrunesALotWindow::resetBlackAiEngine()
{
	if (aiEngineBlack)
	{
		delete aiEngineBlack;
	}

	if (blackPlayerBasicAlphaBeta->isChecked())
	{
		aiEngineBlack = new BasicAlphaBeta();
	}
	else if (blackPlayerAlphaBetaTT->isChecked())
	{
		aiEngineBlack = new AlphaBetaTT();
	}
}

void SerPrunesALotWindow::resetWhiteAiEngine()
{
	if (aiEngineWhite)
	{
		delete aiEngineWhite;
	}

	if (whitePlayerBasicAlphaBeta->isChecked())
	{
		aiEngineWhite = new BasicAlphaBeta();
	}
	else if (whitePlayerAlphaBetaTT->isChecked())
	{
		aiEngineWhite = new AlphaBetaTT();
	}
}

void SerPrunesALotWindow::undoLastMove()
{
	if (movesPlayed.size() > 0)
	{
		Move& move = movesPlayed.back();
		movesPlayed.pop_back();

		// revert all currently highlighted buttons back to their normal color
		for (GameBoardButton* highlighted : highlightedButtons)
		{
			highlighted->resetBackgroundColor();
		}
		highlightedButtons.clear();

		// undo the move and update GUI status
		currentGameState.undoMove(move);
		updateGui();

		// make the squares we came from and went to blue
		boardButtons[move.from.y][move.from.x]->setStyleSheet("background-color:blue;");
		highlightedButtons.push_back(boardButtons[move.from.y][move.from.x]);
		boardButtons[move.to.y][move.to.x]->setStyleSheet("background-color:blue;");
		highlightedButtons.push_back(boardButtons[move.to.y][move.to.x]);

		selectedButton = nullptr;
	}
}

void SerPrunesALotWindow::updateGui()
{
	for (int y = 0; y < BOARD_HEIGHT; ++y)
	{
		for (int x = 0; x < BOARD_WIDTH; ++x)
		{
			EPlayerColors::Type occupant = currentGameState.getOccupier(BoardLocation(x, y));
			GameBoardButton* button = boardButtons[y][x];

			if (occupant == EPlayerColors::NOTHING)
			{
				button->setIcon(QIcon());
			}
			else if (occupant == EPlayerColors::BLACK_PLAYER)
			{
				button->setIcon(QIcon(":/Resources/Resources/BlackIcon.png"));
				button->setIconSize(0.75f * button->size());
			}
			else if (occupant == EPlayerColors::WHITE_PLAYER)
			{
				button->setIcon(QIcon(":/Resources/Resources/WhiteIcon.png"));
				button->setIconSize(0.75f * button->size());
			}
		}
	}
}