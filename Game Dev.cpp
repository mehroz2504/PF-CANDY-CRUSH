// =====================================================================
//  CANDY CRUSH  -  CS1002 Programming Fundamentals, Fall 2026
//  Language: C++   Library: SFML 3 only
//
//  Team:  Member 1 - Abdul Moiz Khan
//         Member 2 - Bisma Fatma
//         Member 3 - Mehruz Shehzad
//
//  HOW THIS TEMPLATE WORKS
//  -----------------------
//  * Everything that is only DRAWING (window, shapes, text, HUD, menu
//    screens) and READING the mouse/keys is already done for you. The
//    project PDF allows AI help for exactly that.
//  * The GAME LOGIC is NOT written. Each piece of logic is an empty
//    function inside a box that looks like this:
//
//        >>>>>  FEATURE 5 : CLEARING, FALLING AND REFILLING  <<<<<
//
//    Inside each box you will find: what to do, how to think about it,
//    and the marks. Write your code where you see  "// TODO".
//  * Suggested split (change it if you like, but everyone must be able
//    to explain EVERY part in the demo):
//        Member 1 (Abdul Moiz): Features 1, 2, 3
//        Member 2 (Bisma)     : Features 4, 5
//        Member 3 (Mehruz)    : Features 6, 7, 8 (+ Bonus)
//    Everyone: Feature 9 (GitHub: 5+ commits each, on 7+ days) and
//    Feature 10 (code quality).
//  * Search for the word  TEMPORARY  - those few lines only exist so the
//    template runs today. DELETE them when you write the real feature.
// =====================================================================

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <string>
using namespace std;

// ---------------------------------------------------------------------
//  CONSTANTS  (Section 4 "Game values" of the PDF - Feature 10: use
//  named constants, never magic numbers like 70 or 8 in your code)
// ---------------------------------------------------------------------
const int   ROWS = 8;
const int   COLS = 8;
const float CELL = 70.f;                 // one cell is 70 x 70 pixels
const float BOARD_X = 20.f;              // top-left corner of the board
const float BOARD_Y = 100.f;
const int   WINDOW_W = 600;
const int   WINDOW_H = 680;
const int   HUD_HEIGHT = 80;             // strip at the top of the window
const int   POINTS_PER_CANDY = 10;       // chains included
const int   NUM_CANDY_TYPES = 6;         // candies are 1..6, 0 = empty

const char* LEVEL_FILE = "level.txt";
const char* SAVE_FILE = "savegame.txt";

// Game states (Feature 7). The game is in exactly ONE of these at a time.
const int MENU = 0;
const int PLAYING = 1;
const int GAME_OVER = 2;                 // used for both "GAME OVER" and "YOU WIN"

// Menu options (Feature 7), in this order
const int MENU_NEW_GAME = 0;
const int MENU_CONTINUE = 1;
const int MENU_EXIT = 2;
const int MENU_OPTION_COUNT = 3;

// Keys that handleInput() can report (so the rest of the code does not
// need to know anything about SFML events)
const int KEY_NONE = 0;
const int KEY_ESCAPE = 1;
const int KEY_ENTER = 2;
const int KEY_UP = 3;
const int KEY_DOWN = 4;

// Bonus: 5 seconds at 60 frames per second
const int HINT_AFTER_FRAMES = 300;

// ---------------------------------------------------------------------
//  GLOBAL VARIABLES  (shared by all features - everyone uses these names)
// ---------------------------------------------------------------------
int  board[ROWS][COLS];                  // 0 = empty, 1..6 = candy colour
bool marked[ROWS][COLS];                 // Feature 4: which candies are part of a match

int  score = 0;
int  moves = 0;                          // moves left
int  target = 0;                         // target score
int  selRow = -1, selCol = -1;           // selected candy, -1 = nothing selected

int  gameState = MENU;
int  menuChoice = MENU_NEW_GAME;         // which menu line is highlighted
bool showNoSaveMsg = false;              // "No saved game" on the menu (Feature 8)
bool levelError = false;                 // level.txt missing (Feature 2)
bool playerWon = false;                  // true = "YOU WIN", false = "GAME OVER"
bool exitRequested = false;              // set to true by the Exit menu option

// Filled by handleInput() every frame, used by updateGame()
bool clicked = false;
int  clickX = 0, clickY = 0;
int  pressedKey = KEY_NONE;

// Bonus (Hint and Shuffle)
int  idleFrames = 0;                     // frames since the player last did something
bool hintActive = false;                 // is a hint being shown?
int  hintR1 = 0, hintC1 = 0, hintR2 = 0, hintC2 = 0;   // the two candies of the hint
bool shuffling = false;                  // show "Shuffling..." on screen?

sf::Font gFont;                          // loaded once in main()

// ---------------------------------------------------------------------
//  FUNCTION PROTOTYPES  (the list of everything in this file)
// ---------------------------------------------------------------------
// Feature 1
void handleInput(sf::RenderWindow& window);
void updateGame();
void drawGame(sf::RenderWindow& window);
// Feature 2
bool loadLevel();
// Feature 3
bool cellFromClick(int x, int y, int& row, int& col);
void handleBoardClick(int x, int y);
bool trySwap(int r1, int c1, int r2, int c2);
// Feature 4
int  findMatches();
// Feature 5
void removeMarked();
void dropCandies();
void refillBoard();
void resolveChain();
// Feature 6
void checkWinLose();
// Feature 7
void startNewGame();
void updateMenu();
void updatePlaying();
void updateGameOver();
// Feature 8
void saveGame();
bool loadGame();
void clearSave();
// Bonus
bool findWorkingSwap(int& r1, int& c1, int& r2, int& c2);
void shuffleBoard();
// Drawing helpers (already done)
void drawText(sf::RenderWindow& window, const string& str, float x, float y,
    unsigned int size, sf::Color color, bool centered = false);
void drawHUD(sf::RenderWindow& window);
void drawBoard(sf::RenderWindow& window);
void drawMenu(sf::RenderWindow& window);
void drawEndScreen(sf::RenderWindow& window);


// =====================================================================
//  main()  -  kept SHORT on purpose (Feature 10).  Already done.
// =====================================================================
int main()
{
    srand((unsigned)time(0));            // random numbers: rand() % n gives 0..n-1

    // SFML needs a font file to draw text. Arial exists on every Windows PC.
    if (!gFont.openFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        cout << "Could not load the font C:/Windows/Fonts/arial.ttf" << endl;
        return 1;
    }

    sf::RenderWindow window(sf::VideoMode({ WINDOW_W, WINDOW_H }), "Candy Crush");
    window.setFramerateLimit(60);        // same speed on every laptop

    // THE GAME LOOP: input -> update -> draw, 60 times every second
    while (window.isOpen())
    {
        handleInput(window);
        updateGame();
        drawGame(window);

        if (exitRequested)
            window.close();
    }
    return 0;
}


// =====================================================================
//  >>>>>  FEATURE 1 : GAME WINDOW AND GAME LOOP  (5 marks)  <<<<<
//  Owner: Member 1
// ---------------------------------------------------------------------
//  The window (600 x 680, "Candy Crush", 60 fps) and the loop in main()
//  are READY. What is left for you:
//   - Read and understand the three functions below. In the demo you
//     must be able to explain what each one does and why they run in
//     this order (input -> update -> draw).
//   - handleInput() only READS events and stores them in the global
//     variables (clicked, clickX, clickY, pressedKey). It does not
//     change the game. That job belongs to updateGame().
//   - Make sure closing the window with the X button ends the program
//     (it does: Closed event -> window.close()).
//  Checklist: window + frame limit (2), three functions in order (2),
//  X button closes properly (1).
// =====================================================================

// JOB 1 of the loop: read what the player did.
void handleInput(sf::RenderWindow& window)
{
    clicked = false;                     // forget last frame's click and key
    pressedKey = KEY_NONE;

    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }
        else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouse->button == sf::Mouse::Button::Left)
            {
                clickX = mouse->position.x;   // where the player clicked, in pixels
                clickY = mouse->position.y;
                clicked = true;
            }
        }
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Escape)     pressedKey = KEY_ESCAPE;
            else if (key->code == sf::Keyboard::Key::Enter) pressedKey = KEY_ENTER;
            else if (key->code == sf::Keyboard::Key::Up)    pressedKey = KEY_UP;
            else if (key->code == sf::Keyboard::Key::Down)  pressedKey = KEY_DOWN;
        }
    }
}

// JOB 2 of the loop: change the game. Each state only does its own work.
void updateGame()
{
    switch (gameState)
    {
    case MENU:      updateMenu();      break;
    case PLAYING:   updatePlaying();   break;
    case GAME_OVER: updateGameOver();  break;
    }
}

// JOB 3 of the loop: draw everything for the current state.
void drawGame(sf::RenderWindow& window)
{
    window.clear(sf::Color(15, 15, 35));          // dark blue background

    switch (gameState)
    {
    case MENU:
        drawMenu(window);
        break;
    case PLAYING:
        drawHUD(window);
        drawBoard(window);
        if (shuffling)
            drawText(window, "Shuffling...", WINDOW_W / 2.f, 40, 28, sf::Color::White, true);
        break;
    case GAME_OVER:
        drawEndScreen(window);
        break;
    }

    window.display();                             // show the finished picture
}


// =====================================================================
//  >>>>>  FEATURE 2 : BOARD FROM A TEXT FILE  (15 marks)  <<<<<
//  Owner: Member 1
// ---------------------------------------------------------------------
//  Read level.txt into board[ROWS][COLS], moves and target.
//  File format (see PDF):
//      line 1: rows columns          (8 8)
//      line 2: moves target          (20 800)
//      then 8 lines of 8 numbers     (0 = empty, 1..6 = candy colour)
//
//  HOW TO THINK ABOUT IT
//   1. Open an ifstream on LEVEL_FILE. If it did not open, return false
//      (the menu then shows an error message instead of crashing).
//   2. Read the four numbers first: rows, cols, moves, target.
//   3. Two nested loops: for r, for c:  fin >> board[r][c];
//   4. Return true.
//
//  Also in this feature (already done for you in drawBoard below, but
//  you must understand it): the board is DRAWN with nested loops, a dark
//  square for every cell, and a colour for every candy number.
//
//  IMPORTANT: put level.txt in the same folder as the .vcxproj file
//  (Visual Studio runs the game from there), not next to main.cpp's
//  parent folder.
//  Checklist: loaded from file, nothing hard-coded (5), drawn with nested
//  loops + visible grid (5), each number has its own colour (3),
//  missing file = error message, no crash (2).
// =====================================================================
bool loadLevel()
{
    ifstream fin(LEVEL_FILE);
    if (!fin.is_open())
    {
        return false;                    // menu shows the error message
    }

    int fileRows, fileCols, fileMoves, fileTarget;
    fin >> fileRows >> fileCols >> fileMoves >> fileTarget;

    // header must be readable and match our board size
    if (fin.fail() || fileRows != ROWS || fileCols != COLS ||
        fileMoves <= 0 || fileTarget <= 0)
    {
        return false;
    }

    // read into a temporary board first, so a bad file cannot ruin board[][]
    int temp[ROWS][COLS];
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            fin >> temp[r][c];
            if (fin.fail() || temp[r][c] < 0 || temp[r][c] > NUM_CANDY_TYPES)
            {
                return false;            // missing or invalid number
            }
        }
    }
    fin.close();

    // everything is valid, so copy it into the real board
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            board[r][c] = temp[r][c];

    moves = fileMoves;
    target = fileTarget;
    return true;
}


// =====================================================================
//  >>>>>  FEATURE 3 : SELECTING AND SWAPPING  (10 marks)  <<<<<
//  Owner: Member 1
// ---------------------------------------------------------------------
//  The player uses the mouse. selRow / selCol hold the selected candy
//  (-1 means nothing selected). drawBoard() already draws the white ring
//  around the selected candy - you only have to set selRow / selCol.
//
//  A) cellFromClick(x, y, row, col)
//     Turn pixels into a cell. Return false if the click is outside the
//     board (x < BOARD_X or x >= BOARD_X + COLS*CELL, same for y).
//        col = (x - BOARD_X) / CELL      row = (y - BOARD_Y) / CELL
//
//  B) handleBoardClick(x, y)   - the rules:
//     - click outside the board          -> ignore
//     - first click (nothing selected)   -> select that candy
//     - second click on a NEIGHBOUR      -> trySwap(...)
//         neighbour means  abs(r1-r2) + abs(c1-c2) == 1  (never diagonal)
//     - second click anywhere else       -> select that candy instead
//     - after a swap, nothing is selected (selRow = selCol = -1)
//
//  C) trySwap(r1, c1, r2, c2)  - returns true if the swap was kept
//     1. swap the two candies
//     2. if findMatches() == 0  -> swap them BACK, return false.
//        This does NOT use a move.
//     3. otherwise it is a good swap:
//          moves--;                     (Feature 6: exactly one move)
//          resolveChain();              (Feature 5)
//          checkWinLose();              (Feature 6)
//          return true.
//  Checklist: select + ring + ignore outside (3), neighbour swap only (3),
//  no-match swap goes back and costs no move (4).
// =====================================================================
bool cellFromClick(int x, int y, int& row, int& col)
{
    // TODO (Feature 3): return false if (x, y) is outside the board.
    // TODO (Feature 3): otherwise compute row and col and return true.
    return false;
}

void handleBoardClick(int x, int y)
{
    // TODO (Feature 3): use cellFromClick, then the rules from the box above.
}

bool trySwap(int r1, int c1, int r2, int c2)
{
    int temp = board[row1][col1];
    board[row1][col1] = board[row2][col2];
    board[row2][col2] = temp;
}


// =====================================================================
//  >>>>>  FEATURE 4 : FINDING MATCHES  (10 marks)  <<<<<
//  Owner: Member 2
// ---------------------------------------------------------------------
//  One function: int findMatches()
//  It fills marked[][] and RETURNS how many candies were marked.
//  It is used after every swap, after every fall, and in the bonus.
//
//  HOW TO THINK ABOUT IT
//   1. Set the whole marked[][] array to false.
//   2. ROWS: for every r, and every c from 0 to COLS-3:
//        if board[r][c] != 0 and board[r][c] == board[r][c+1]
//           == board[r][c+2]  -> mark all three cells.
//   3. COLUMNS: the same, with r, r+1, r+2.
//   4. Count the marked cells and return the count.
//
//  DO NOT remove anything while checking, or you will miss L and T
//  shapes. A line of 4 or 5 is just overlapping groups of 3, so it gets
//  marked completely with no extra code.
//  Checklist: row matches (3), column matches (3), all candies marked
//  incl. 4/5 lines and L/T shapes (4).
// =====================================================================
int findMatches()
{
    // TODO (Feature 4): clear marked[][].
    // TODO (Feature 4): check rows, then columns, mark groups of three.
    // TODO (Feature 4): count and return the number of marked candies.
    return 0;
}


// =====================================================================
//  >>>>>  FEATURE 5 : CLEARING, FALLING AND REFILLING  (25 marks)  <<<<<
//  Owner: Member 2   (the biggest feature - Members 1 and 3 should help
//  test it, and everyone must understand it)
// ---------------------------------------------------------------------
//  After a swap that makes a match, three things happen, then we repeat.
//
//  A) removeMarked()
//     Every candy with marked[r][c] == true: set board[r][c] = 0 and
//     add POINTS_PER_CANDY to score. (10 points for each candy, chains
//     included.)
//
//  B) dropCandies()   - one COLUMN at a time
//     for each column c:
//        writeRow = ROWS - 1                  (bottom row)
//        go from the bottom row UP (r = ROWS-1 down to 0):
//           if board[r][c] != 0: move it to board[writeRow][c], then
//           writeRow--
//        every cell above writeRow (row 0 .. writeRow) becomes 0.
//     Be careful: a candy may already be in the right place, and
//     columns can have MORE THAN ONE gap.
//
//  C) refillBoard()
//     Every cell that is still 0 gets  rand() % NUM_CANDY_TYPES + 1.
//
//  D) resolveChain()   - the chain loop
//     New and fallen candies can make new matches by themselves, so:
//        while (findMatches() > 0)
//        {
//            removeMarked();
//            dropCandies();
//            refillBoard();
//        }
//     A chain uses only ONE move in total (moves-- happens once, in
//     trySwap), but every candy in it gives 10 points.
//     (Optional, not marked: wait a few frames between the rounds so the
//     player can see the chain.)
//
//  Checklist: remove + 10 points each (5), correct falling incl. several
//  gaps (8), refill at the top (4), chains repeat and count points (8).
// =====================================================================
void removeMarked()
{
    // TODO (Feature 5): set marked candies to 0 and add the points.
}

void dropCandies()
{
    // TODO (Feature 5): for every column, move candies down using writeRow.
}

void refillBoard()
{
    // TODO (Feature 5): fill every 0 with a random candy 1..6.
}

void resolveChain()
{
    // TODO (Feature 5): while there are matches: remove, drop, refill.
}


// =====================================================================
//  >>>>>  FEATURE 6 : SCORE, MOVES AND WINNING  (10 marks)  <<<<<
//  Owner: Member 3
// ---------------------------------------------------------------------
//  The HUD (score, moves left, target) is already DRAWN by drawHUD(). It
//  reads the globals score / moves / target every frame, so it updates
//  straight away as soon as your logic changes those variables.
//  What you write:
//   - moves: loaded from level.txt (Feature 2); each swap that makes a
//     match uses exactly ONE move (moves-- in trySwap, Feature 3). A
//     swap that is reverted, or a chain, never uses a second move.
//   - checkWinLose(), called after a move AND all its chains are done:
//        if score >= target  -> playerWon = true,  game ends
//        else if moves == 0  -> playerWon = false, game ends
//     "Game ends" means: clearSave() (Feature 8) and
//     gameState = GAME_OVER (Feature 7).
//  Checklist: HUD updates straight away (4), exactly one move per good
//  swap (3), YOU WIN / GAME OVER (3).
// =====================================================================
void checkWinLose()
{
        // target reached, the player wins
        if (score >= target)
        {
            playerWon = true;
            clearSave();
            gameState = GAME_OVER;
        }
       
}


// =====================================================================
//  >>>>>  FEATURE 7 : MENU AND GAME STATES  (8 marks)  <<<<<
//  Owner: Member 3
// ---------------------------------------------------------------------
//  gameState is MENU, PLAYING or GAME_OVER. updateGame() already uses
//  switch (gameState) and calls one of the three functions below.
//  Drawing of all three screens is already done (drawMenu, drawHUD +
//  drawBoard, drawEndScreen). You write the behaviour:
//
//  startNewGame()   - "New Game always starts fresh"
//     score = 0, selRow = selCol = -1, load board/moves/target from
//     level.txt (loadLevel). If loadLevel() returns false: set
//     levelError = true and STAY on the menu. Otherwise levelError =
//     false and gameState = PLAYING.
//
//  updateMenu()     - three options in this order: New Game, Continue, Exit
//     KEY_UP / KEY_DOWN change menuChoice (stay between 0 and 2).
//     KEY_ENTER:
//        MENU_NEW_GAME -> startNewGame()
//        MENU_CONTINUE -> loadGame() (Feature 8); if it returns false
//                         set showNoSaveMsg = true and stay on the menu
//        MENU_EXIT     -> exitRequested = true
//
//  updatePlaying()  - if (clicked) handleBoardClick(clickX, clickY);
//     if (pressedKey == KEY_ESCAPE) saveGame() then gameState = MENU
//     (Feature 8).  Bonus: hint/shuffle checks also go here.
//
//  updateGameOver() - KEY_ENTER -> gameState = MENU.
//     (The save is already emptied when the game ended.)
//
//  Checklist: gameState + named constants + switch (2), menu works and
//  New Game starts fresh (4), end screen + Enter goes to menu (2).
// =====================================================================
void startNewGame()
{
    // at the start
    score = 0;
    selRow = selCol = -1;
    // so that ring is back at where it was at last 

    if (loadLevel())
    {
        levelError = false;
        gameState = PLAYING;
    }
    else
    {
        levelError = true;
    }
    //if level file opens it says true else false

}

void updateMenu()
{
    // del old msg when user presses a key
    if (pressedKey != KEY_NONE)
    {
        showNoSaveMsg = false;
        levelError = false; // these msg get deleted when user presses any key
    }

    // move up
    if (pressedKey == KEY_UP && menuChoice > 0)
    {
        menuChoice--;
    }
    //it means if key is pressed upward and menu choice is greater than 0 so move up

    // move down
    if (pressedKey == KEY_DOWN && menuChoice < 2)
    {
        menuChoice++;
    }
    //it means if key is pressed downward and menu choice is greater than 0 so move down

    // enter pressed, do what is selected
    if (pressedKey == KEY_ENTER)
    {
        if (menuChoice == MENU_NEW_GAME)
        {
            startNewGame();
        }
        //if command is new game so start a new game
        else if (menuChoice == MENU_CONTINUE)
        {
            // if there is no save, show the message
            if (loadGame() == false)
            {
                showNoSaveMsg = true;
            }
        }
        //if command is continue so just load game but game is only loaded if there is a saved game present
        else if (menuChoice == MENU_EXIT)
        {
            exitRequested = true;
        }
        //if it orders exit so exit game
    }
}


void updatePlaying()
{

    // if user taps somewhere on screen so let board handle it
    if (clicked == true)
    {
        handleBoardClick(clickX, clickY);
    }
    else if (pressedKey == KEY_ESCAPE)
    {
        saveGame();
        gameState = MENU;
    }
    //if user presses esc just save the game

    // TODO (Bonus): hint timer and shuffle check
}

void updateGameOver()
{
    // if user perse enter go back to menu
    if (pressedKey == KEY_ENTER)
    {
        gameState = MENU;
    }

}


// =====================================================================
//  >>>>>  FEATURE 8 : SAVE AND CONTINUE  (7 marks)  <<<<<
//  Owner: Member 3
// ---------------------------------------------------------------------
//  savegame.txt format (do not change it):
//      line 1:   score moves target        e.g.  640 12 800
//      lines 2-9: the 8 board rows, 8 numbers each, separated by spaces
//  Do NOT save the selected candy. After Continue nothing is selected.
//
//  saveGame()   - ofstream on SAVE_FILE; write the line 1, then the board
//                 with two nested loops. Called when Esc is pressed while
//                 playing (then go back to the menu).
//  loadGame()   - ifstream on SAVE_FILE. If the file cannot be opened OR
//                 is empty (a finished game), return false. Otherwise
//                 read score, moves, target and the board, set
//                 selRow = selCol = -1, gameState = PLAYING, return true.
//                 Hint: an empty file makes  fin >> score  fail - check it.
//  clearSave()  - open SAVE_FILE with ofstream and close it straight away
//                 (that empties it). Call it when a game ends
//                 (win or game over - inside checkWinLose, Feature 6).
//  On the menu, when loadGame() returns false: showNoSaveMsg = true.
//  Checklist: Esc saves in the format (3), Continue restores exactly (3),
//  "No saved game" shown + save emptied when a game ends (1).
// =====================================================================
void saveGame()
{
    //opens save file
    ofstream fout(SAVE_FILE);
    //if file does not open
    if (!fout.is_open())
    {
        return;
    }
    // score,moves left,target
    fout << score << " " << moves << " " << target << endl;
    //next lines for board 
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            fout << board[r][c];
            //space btw number but not at last
            if (c < COLS - 1)
            {
                fout << " ";
            }
        }
        fout << endl;//endl so that first line gets separated from board
    }
    fout.close();
}

bool loadGame()
{
    // open the save file for reading
    ifstream fin(SAVE_FILE);

    // no file means no saved game
    if (!fin.is_open())
    {
        return false;
    }

    // read the first line: score, moves left, target
    int savedScore, savedMoves, savedTarget;
    fin >> savedScore >> savedMoves >> savedTarget;

    // an empty file makes this fail
    if (fin.fail())
    {
        return false;
    }

    // read the board into a temporary array first, so a bad file
    // cannot damage the real board
    int temp[ROWS][COLS];
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            fin >> temp[r][c];
            if (fin.fail() || temp[r][c] < 0 || temp[r][c] > NUM_CANDY_TYPES)
            {
                return false;
            }
        }
    }
    fin.close();

    // everything is fine so now we can copy the values
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            board[r][c] = temp[r][c];

    score = savedScore;
    moves = savedMoves;
    target = savedTarget;

    // nothing is selected after Continue
    selRow = -1;
    selCol = -1;
    gameState = PLAYING;

    return true;
}

void clearSave()
{
    ofstream fout(SAVE_FILE);            // opening for writing empties the file
    fout.close();
}


// =====================================================================
//  >>>>>  BONUS : HINT AND SHUFFLE  (+5 marks)  <<<<<
//  Owner: Member 3 (only after the 10 features work!)
// ---------------------------------------------------------------------
//  Both need the same thing: find a swap that works.
//  findWorkingSwap(r1, c1, r2, c2):
//     go through every cell; try it with its RIGHT neighbour and its DOWN
//     neighbour: swap, call findMatches(), swap back. If findMatches()
//     was > 0, store the two cells in the four reference parameters and
//     return true. If nothing works, return false.
//     (Remember to swap BACK every time, or you destroy the board.)
//
//  HINT   : in updatePlaying(), idleFrames++ every frame; reset it to 0
//           when the player clicks. When idleFrames >= HINT_AFTER_FRAMES
//           (300 = 5 seconds) call findWorkingSwap and set hintActive =
//           true with hintR1.. hintC2. A click sets hintActive = false.
//           drawBoard() already draws a yellow highlight when hintActive.
//  SHUFFLE: shuffleBoard() fills the board with random candies and
//           repeats until findMatches() == 0 AND findWorkingSwap() is
//           true. Call it at the start of a game and after every move.
//           Set shuffling = true for a moment so "Shuffling..." shows.
//  Checklist: hint after 5 s, goes away on click (3); stuck board is
//  shuffled with no matches on it (2).
// =====================================================================
bool findWorkingSwap(int& r1, int& c1, int& r2, int& c2)
{
    // TODO (Bonus): try every swap with the right and down neighbour.
    return false;
}

void shuffleBoard()
{
    // TODO (Bonus): refill randomly until no matches and a swap works.
}


// =====================================================================
//  DRAWING  -  ALREADY DONE (graphics only, AI-assisted as the PDF
//  allows). You do not have to change these, but read them: in the demo
//  you may be asked how the board is drawn.
// =====================================================================

// Draws a piece of text. If centered is true, x is the CENTRE of the text.
void drawText(sf::RenderWindow& window, const string& str, float x, float y,
    unsigned int size, sf::Color color, bool centered)
{
    sf::Text text(gFont, str, size);
    text.setFillColor(color);
    if (centered)
    {
        sf::FloatRect b = text.getLocalBounds();
        text.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
    }
    text.setPosition({ x, y });
    window.draw(text);
}

// The 80-pixel strip at the top: SCORE, MOVES, TARGET.
void drawHUD(sf::RenderWindow& window)
{
    sf::RectangleShape panel({ WINDOW_W - 40.f, HUD_HEIGHT - 20.f });
    panel.setPosition({ 20.f, 10.f });
    panel.setFillColor(sf::Color(25, 25, 55));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(90, 90, 150));
    window.draw(panel);

    float third = WINDOW_W / 3.f;
    sf::Color label(150, 150, 190);

    drawText(window, "SCORE", third * 0.5f, 16, 14, label, true);
    drawText(window, to_string(score), third * 0.5f, 36, 26, sf::Color::White, true);

    drawText(window, "MOVES", third * 1.5f, 16, 14, label, true);
    drawText(window, to_string(moves), third * 1.5f, 36, 26, sf::Color::Yellow, true);

    drawText(window, "TARGET", third * 2.5f, 16, 14, label, true);
    drawText(window, to_string(target), third * 2.5f, 36, 26, sf::Color::Green, true);
}

// The board: a dark square for every cell, the candy on top of it.
void drawBoard(sf::RenderWindow& window)
{
    // One colour for every number that can be in the board
    sf::Color colors[7] = {
        sf::Color(25, 25, 45),           // 0: empty
        sf::Color::Red,                  // 1
        sf::Color(255, 150, 0),          // 2: orange
        sf::Color::Yellow,               // 3
        sf::Color::Green,                // 4
        sf::Color::Blue,                 // 5
        sf::Color(170, 90, 255)          // 6: purple
    };

    sf::RectangleShape square({ CELL, CELL });          // the grid square
    square.setOutlineThickness(1.f);
    square.setOutlineColor(sf::Color(60, 60, 100));

    sf::CircleShape candy(28.f);                        // radius 28 fits a 70 x 70 cell
    sf::CircleShape shine(5.f);                         // small white highlight
    shine.setFillColor(sf::Color(255, 255, 255, 170));

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            float x = BOARD_X + c * CELL;
            float y = BOARD_Y + r * CELL;

            // dark square (two shades, like a chess board, so the grid is visible)
            square.setPosition({ x, y });
            square.setFillColor((r + c) % 2 == 0 ? sf::Color(30, 30, 60) : sf::Color(22, 22, 48));
            window.draw(square);

            // hint highlight (Bonus)
            bool isHint = hintActive &&
                ((r == hintR1 && c == hintC1) || (r == hintR2 && c == hintC2));
            if (isHint)
            {
                sf::RectangleShape glow({ CELL - 6.f, CELL - 6.f });
                glow.setPosition({ x + 3.f, y + 3.f });
                glow.setFillColor(sf::Color::Transparent);
                glow.setOutlineThickness(3.f);
                glow.setOutlineColor(sf::Color::Yellow);
                window.draw(glow);
            }

            // the candy (0 = empty, nothing to draw)
            int type = board[r][c];
            if (type >= 1 && type <= NUM_CANDY_TYPES)
            {
                candy.setFillColor(colors[type]);
                candy.setPosition({ x + 7.f, y + 7.f });

                if (r == selRow && c == selCol)          // white ring around the selected candy
                {
                    candy.setOutlineThickness(4.f);
                    candy.setOutlineColor(sf::Color::White);
                }
                else
                {
                    candy.setOutlineThickness(0.f);
                }
                window.draw(candy);

                shine.setPosition({ x + 20.f, y + 18.f });
                window.draw(shine);
            }
        }
    }
}

// The main menu: New Game, Continue, Exit.
void drawMenu(sf::RenderWindow& window)
{
    drawText(window, "CANDY CRUSH", WINDOW_W / 2.f, 120, 56, sf::Color(255, 90, 130), true);
    drawText(window, "Programming Fundamentals Project", WINDOW_W / 2.f, 195, 18,
        sf::Color(150, 150, 190), true);

    const char* options[MENU_OPTION_COUNT] = { "New Game", "Continue", "Exit" };
    for (int i = 0; i < MENU_OPTION_COUNT; i++)
    {
        bool chosen = (i == menuChoice);
        string line = chosen ? string("> ") + options[i] + " <" : string(options[i]);
        drawText(window, line, WINDOW_W / 2.f, 290.f + i * 60.f, 34,
            chosen ? sf::Color::Yellow : sf::Color::White, true);
    }

    if (showNoSaveMsg)
        drawText(window, "No saved game", WINDOW_W / 2.f, 500, 24, sf::Color(255, 120, 120), true);
    if (levelError)
        drawText(window, "Error: level.txt not found", WINDOW_W / 2.f, 540, 24,
            sf::Color(255, 120, 120), true);

    drawText(window, "Up / Down to choose, Enter to select", WINDOW_W / 2.f, 620, 16,
        sf::Color(150, 150, 190), true);
}

// "YOU WIN" or "GAME OVER" with the final score.
void drawEndScreen(sf::RenderWindow& window)
{
    if (playerWon)
        drawText(window, "YOU WIN", WINDOW_W / 2.f, 200, 64, sf::Color::Green, true);
    else
        drawText(window, "GAME OVER", WINDOW_W / 2.f, 200, 64, sf::Color(255, 80, 80), true);

    drawText(window, "Final score: " + to_string(score), WINDOW_W / 2.f, 320, 32,
        sf::Color::White, true);
    drawText(window, "Press Enter", WINDOW_W / 2.f, 420, 26, sf::Color::Yellow, true);
}