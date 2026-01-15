#include <iostream>
#include <string>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <cstdlib>
#endif

using namespace std;

const int BOARD_SIZE = 8;
const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;
const int MAX_DEPTH = 4;

int board[BOARD_SIZE][BOARD_SIZE];
int moveCount = 0;

// Forward declarations
void countPieces(int &blackCount, int &whiteCount);

void initBoard() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = EMPTY;
        }
    }
    board[3][3] = WHITE;
    board[3][4] = BLACK;
    board[4][3] = BLACK;
    board[4][4] = WHITE;
    moveCount = 4;
}

void clearScreen() {
    #ifdef _WIN32
        // Windows-specific console clearing
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coordScreen = {0, 0};
        DWORD cCharsWritten;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD dwConSize;
        
        if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            return;
        }
        
        dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
        
        FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
        SetConsoleCursorPosition(hConsole, coordScreen);
    #else
        // Linux/Unix - use ANSI escape codes
        cout << "\033[2J\033[1;1H";
        cout.flush();
    #endif
}

void displayBoard() {
    clearScreen();  // Clear previous board display
    
    // Display game title and instructions
    cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    cout << "║              REVERSI (OTHELLO) - AI GAME                      ║\n";
    cout << "╠═══════════════════════════════════════════════════════════════╣\n";
    cout << "║  You are BLACK ⚫  |  AI is WHITE ⚪                          ║\n";
    cout << "║  Enter moves as: A1, B2, C3, etc.                             ║\n";
    cout << "╚═══════════════════════════════════════════════════════════════╝\n";
    
    // Display current score
    int blackCount, whiteCount;
    countPieces(blackCount, whiteCount);
    cout << "\n  Score - Black (⚫): " << blackCount << "  |  White (⚪): " << whiteCount << "\n";
    
    // Top border with column labels
    cout << "\n    A    B    C    D    E    F    G    H\n";
    cout << "  ╔════╦════╦════╦════╦════╦════╦════╦════╗\n";
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        cout << (i + 1) << " ║";
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == BLACK) {
                cout << " ⚫";
            } else if (board[i][j] == WHITE) {
                cout << " ⚪";
            } else {
                cout << "   ";
            }
            cout << " ║";
        }
        cout << " " << "\n";
        
        if (i < BOARD_SIZE - 1) {
            cout << "  ╠════╬════╬════╬════╬════╬════╬════╬════╣\n";
        }
    }
    
    // Bottom border with column labels
    cout << "  ╚════╩════╩════╩════╩════╩════╩════╩════╝\n";
}

bool isInBounds(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

bool checkDirection(int row, int col, int dr, int dc, int player) {
    int opponent = (player == BLACK) ? WHITE : BLACK;
    int r = row + dr;
    int c = col + dc;
    bool foundOpponent = false;
    
    while (isInBounds(r, c)) {
        if (board[r][c] == EMPTY) {
            return false;
        }
        if (board[r][c] == opponent) {
            foundOpponent = true;
        } else if (board[r][c] == player) {
            return foundOpponent;
        }
        r += dr;
        c += dc;
    }
    return false;
}

bool isValidMove(int row, int col, int player) {
    if (!isInBounds(row, col) || board[row][col] != EMPTY) {
        return false;
    }
    
    int directions[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    
    for (int i = 0; i < 8; i++) {
        if (checkDirection(row, col, directions[i][0], directions[i][1], player)) {
            return true;
        }
    }
    return false;
}

void flipDirection(int row, int col, int dr, int dc, int player) {
    int opponent = (player == BLACK) ? WHITE : BLACK;
    int r = row + dr;
    int c = col + dc;
    
    while (isInBounds(r, c) && board[r][c] == opponent) {
        board[r][c] = player;
        r += dr;
        c += dc;
    }
}

void makeMove(int row, int col, int player) {
    board[row][col] = player;
    moveCount++;
    
    int directions[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    
    for (int i = 0; i < 8; i++) {
        if (checkDirection(row, col, directions[i][0], directions[i][1], player)) {
            flipDirection(row, col, directions[i][0], directions[i][1], player);
        }
    }
}

bool hasValidMoves(int player) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (isValidMove(i, j, player)) {
                return true;
            }
        }
    }
    return false;
}

void countPieces(int &blackCount, int &whiteCount) {
    blackCount = 0;
    whiteCount = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == BLACK) blackCount++;
            else if (board[i][j] == WHITE) whiteCount++;
        }
    }
}

int evaluateBoard(int player) {
    int blackCount, whiteCount;
    countPieces(blackCount, whiteCount);
    
    int cornerWeight = 25;
    int edgeWeight = 5;
    int score = 0;
    
    if (player == BLACK) {
        score = blackCount - whiteCount;
    } else {
        score = whiteCount - blackCount;
    }
    
    int corners[4][2] = {{0,0},{0,7},{7,0},{7,7}};
    for (int i = 0; i < 4; i++) {
        int r = corners[i][0];
        int c = corners[i][1];
        if (board[r][c] == player) {
            score += cornerWeight;
        } else if (board[r][c] != EMPTY) {
            score -= cornerWeight;
        }
    }
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board[0][i] == player || board[7][i] == player || 
            board[i][0] == player || board[i][7] == player) {
            score += edgeWeight;
        }
    }
    
    return score;
}

int minimax(int depth, bool isMaximizing, int player, int alpha, int beta) {
    if (depth == 0 || moveCount == BOARD_SIZE * BOARD_SIZE) {
        return evaluateBoard(player);
    }
    
    int opponent = (player == BLACK) ? WHITE : BLACK;
    int currentPlayer = isMaximizing ? player : opponent;
    
    if (!hasValidMoves(currentPlayer)) {
        if (!hasValidMoves(opponent)) {
            return evaluateBoard(player);
        }
        return minimax(depth - 1, !isMaximizing, player, alpha, beta);
    }
    
    if (isMaximizing) {
        int maxEval = -100000;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (isValidMove(i, j, currentPlayer)) {
                    int tempBoard[BOARD_SIZE][BOARD_SIZE];
                    int tempMoveCount = moveCount;
                    for (int x = 0; x < BOARD_SIZE; x++) {
                        for (int y = 0; y < BOARD_SIZE; y++) {
                            tempBoard[x][y] = board[x][y];
                        }
                    }
                    
                    makeMove(i, j, currentPlayer);
                    int eval = minimax(depth - 1, false, player, alpha, beta);
                    
                    for (int x = 0; x < BOARD_SIZE; x++) {
                        for (int y = 0; y < BOARD_SIZE; y++) {
                            board[x][y] = tempBoard[x][y];
                        }
                    }
                    moveCount = tempMoveCount;
                    
                    maxEval = (eval > maxEval) ? eval : maxEval;
                    alpha = (alpha > eval) ? alpha : eval;
                    if (beta <= alpha) break;
                }
            }
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = 100000;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (isValidMove(i, j, currentPlayer)) {
                    int tempBoard[BOARD_SIZE][BOARD_SIZE];
                    int tempMoveCount = moveCount;
                    for (int x = 0; x < BOARD_SIZE; x++) {
                        for (int y = 0; y < BOARD_SIZE; y++) {
                            tempBoard[x][y] = board[x][y];
                        }
                    }
                    
                    makeMove(i, j, currentPlayer);
                    int eval = minimax(depth - 1, true, player, alpha, beta);
                    
                    for (int x = 0; x < BOARD_SIZE; x++) {
                        for (int y = 0; y < BOARD_SIZE; y++) {
                            board[x][y] = tempBoard[x][y];
                        }
                    }
                    moveCount = tempMoveCount;
                    
                    minEval = (eval < minEval) ? eval : minEval;
                    beta = (beta < eval) ? beta : eval;
                    if (beta <= alpha) break;
                }
            }
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

void getAIMove(int &row, int &col, int player) {
    int bestScore = -100000;
    int bestRow = -1;
    int bestCol = -1;
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (isValidMove(i, j, player)) {
                int tempBoard[BOARD_SIZE][BOARD_SIZE];
                int tempMoveCount = moveCount;
                for (int x = 0; x < BOARD_SIZE; x++) {
                    for (int y = 0; y < BOARD_SIZE; y++) {
                        tempBoard[x][y] = board[x][y];
                    }
                }
                
                makeMove(i, j, player);
                int score = minimax(MAX_DEPTH - 1, false, player, -100000, 100000);
                
                for (int x = 0; x < BOARD_SIZE; x++) {
                    for (int y = 0; y < BOARD_SIZE; y++) {
                        board[x][y] = tempBoard[x][y];
                    }
                }
                moveCount = tempMoveCount;
                
                if (score > bestScore) {
                    bestScore = score;
                    bestRow = i;
                    bestCol = j;
                }
            }
        }
    }
    
    row = bestRow;
    col = bestCol;
}

int main() {
    // Set console to UTF-8 for proper Unicode character display
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif
    // Linux/Unix typically use UTF-8 by default
    
    initBoard();
    int currentPlayer = BLACK;
    bool gameOver = false;
    
    while (!gameOver) {
        displayBoard();
        
        if (!hasValidMoves(currentPlayer)) {
            if (!hasValidMoves((currentPlayer == BLACK) ? WHITE : BLACK)) {
                gameOver = true;
                break;
            }
            cout << "\n⚠️  " << ((currentPlayer == BLACK) ? "Black" : "White") << " has no valid moves. Passing...\n";
            cout << "Press Enter to continue...";
            cin.ignore();
            cin.get();
            currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
            continue;
        }
        
        if (currentPlayer == BLACK) {
            cout << "\nYour turn (BLACK): ";
            string move;
            cin >> move;
            
            if (move.length() < 2) {
                cout << "Invalid input!\n";
                continue;
            }
            
            int col = move[0] - 'A';
            if (move[0] >= 'a') col = move[0] - 'a';
            int row = move[1] - '1';
            
            if (!isValidMove(row, col, currentPlayer)) {
                cout << "Invalid move! Try again.\n";
                continue;
            }
            
            makeMove(row, col, currentPlayer);
        } else {
            cout << "\nAI is thinking...\n";
            int row, col;
            getAIMove(row, col, currentPlayer);
            makeMove(row, col, currentPlayer);
            cout << "AI played: " << (char)('A' + col) << (row + 1) << "\n";
        }
        
        currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
    }
    
    displayBoard();
    
    int blackCount, whiteCount;
    countPieces(blackCount, whiteCount);
    
    cout << "\n=== GAME OVER ===\n";
    cout << "Final Score:\n";
    cout << "Black (●): " << blackCount << "\n";
    cout << "White (○): " << whiteCount << "\n";
    
    if (blackCount > whiteCount) {
        cout << "\nYOU WIN!\n";
    } else if (whiteCount > blackCount) {
        cout << "\nAI WINS!\n";
    } else {
        cout << "\nIT'S A TIE!\n";
    }
    
    return 0;
}
