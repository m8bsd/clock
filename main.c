#include <stdio.h>

char board[3][3];
char currentPlayer = 'X';

/* Function to initialize the board */
void initializeBoard() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = ' ';
}

/* Function to print the board */
void printBoard() {
    printf("\n");
    for (int i = 0; i < 3; i++) {
        printf(" %c | %c | %c ", board[i][0], board[i][1], board[i][2]);
        if (i < 2)
            printf("\n---|---|---\n");
    }
    printf("\n\n");
}

/* Check if current player won */
int checkWin() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == currentPlayer &&
            board[i][1] == currentPlayer &&
            board[i][2] == currentPlayer)
            return 1;

        if (board[0][i] == currentPlayer &&
            board[1][i] == currentPlayer &&
            board[2][i] == currentPlayer)
            return 1;
    }

    if (board[0][0] == currentPlayer &&
        board[1][1] == currentPlayer &&
        board[2][2] == currentPlayer)
        return 1;

    if (board[0][2] == currentPlayer &&
        board[1][1] == currentPlayer &&
        board[2][0] == currentPlayer)
        return 1;

    return 0;
}

/* Check for draw */
int checkDraw() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ')
                return 0;
    return 1;
}

/* Switch players */
void switchPlayer() {
    if (currentPlayer == 'X')
        currentPlayer = 'O';
    else
        currentPlayer = 'X';
}

int main() {
    int row, col;
    initializeBoard();

    printf("🎮 Tic Tac Toe Game\n");
    printf("Player X vs Player O\n");

    while (1) {
        printBoard();
        printf("Player %c, enter row (1-3) and column (1-3): ", currentPlayer);
        scanf("%d %d", &row, &col);

        row--; col--;

        if (row < 0 || row > 2 || col < 0 || col > 2 || board[row][col] != ' ') {
            printf("❌ Invalid move! Try again.\n");
            continue;
        }

        board[row][col] = currentPlayer;

        if (checkWin()) {
            printBoard();
            printf("🏆 Player %c wins!\n", currentPlayer);
            break;
        }

        if (checkDraw()) {
            printBoard();
            printf("🤝 It's a draw!\n");
            break;
        }

        switchPlayer();
    }

    return 0;
}
