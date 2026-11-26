#include <SFML/Graphics.hpp>
using namespace std;

const int BOARD_X = 100;
const int BOARD_Y = 80;
const int CELL_SIZE = 60;

int selectedRow = -1;
int selectedCol = -1;

int findMatches(const int board[8][8], bool marked[8][8]);

void swapCandies(int board[8][8], int row1, int col1, int row2, int col2)
{
    int temp = board[row1][col1];
    board[row1][col1] = board[row2][col2];
    board[row2][col2] = temp;
}

void handleCandyClick(int mouseX, int mouseY, int board[8][8], int& moves)
{
    if (mouseX < BOARD_X || mouseX >= BOARD_X + 8 * CELL_SIZE ||
        mouseY < BOARD_Y || mouseY >= BOARD_Y + 8 * CELL_SIZE)
    {
        return;
    }

    int col = (mouseX - BOARD_X) / CELL_SIZE;
    int row = (mouseY - BOARD_Y) / CELL_SIZE;

    if (board[row][col] == 0)
    {
        return;
    }

    if (selectedRow == -1)
    {
        selectedRow = row;
        selectedCol = col;
        return;
    }
    int rowDistance = row - selectedRow;
    int colDistance = col - selectedCol;

    if (rowDistance < 0)
        rowDistance = -rowDistance;

    if (colDistance < 0)
        colDistance = -colDistance;

    if (rowDistance + colDistance != 1)
    {
        selectedRow = row;
        selectedCol = col;
        return;
    }

    swapCandies(board, selectedRow, selectedCol, row, col);


    bool marked[8][8] = {};
    findMatches(board, marked);

    bool matchBana = marked[selectedRow][selectedCol] ||
        marked[row][col];

    if (matchBana)
    {
        moves++;
    }
    else
    {
        swapCandies(board, selectedRow, selectedCol, row, col);
    }

    selectedRow = -1;
    selectedCol = -1;
}

void drawSelectedCandy(sf::RenderWindow& window)
{
    if (selectedRow == -1)
    {
        return;
    }

    sf::RectangleShape border(
        sf::Vector2f(
            static_cast<float>(CELL_SIZE - 4),
            static_cast<float>(CELL_SIZE - 4)
        )
    );

    border.setPosition(sf::Vector2f(
        static_cast<float>(BOARD_X + selectedCol * CELL_SIZE + 2),
        static_cast<float>(BOARD_Y + selectedRow * CELL_SIZE + 2)
    ));

    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(3.f);

    window.draw(border);
}