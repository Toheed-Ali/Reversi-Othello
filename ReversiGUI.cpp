#include <iostream>
#include <string>
#include <cmath>
#include "raylib.h"

using namespace std;

const int BOARD_SIZE = 8;
const int EMPTY = 0;
const int PLAYER_BLACK = 1;
const int PLAYER_WHITE = 2;
const int MAX_DEPTH = 4;

const int CELL_SIZE = 80;
const int BOARD_OFFSET_X = 84;
const int BOARD_OFFSET_Y = 150;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 850;
const float ANIMATION_DURATION = 0.5f; // seconds

int board[BOARD_SIZE][BOARD_SIZE];
int moveCount = 0;
bool gameOver = false;
int currentPlayer = PLAYER_BLACK;

// Animation system - using arrays instead of struct
int animRow[64];
int animCol[64];
int animFromPlayer[64];
int animToPlayer[64];
float animStartTime[64];
float animProgress[64];
int animationCount = 0;
bool isAnimating = false;
float gameTime = 0.0f;

// Forward declarations
void countPieces(int &blackCount, int &whiteCount);
bool hasValidMoves(int player);
void getAIMove(int &row, int &col, int player);
void makeMove(int row, int col, int player);
bool isValidMove(int row, int col, int player);

void initBoard() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = EMPTY;
        }
    }
    board[3][3] = PLAYER_WHITE;
    board[3][4] = PLAYER_BLACK;
    board[4][3] = PLAYER_BLACK;
    board[4][4] = PLAYER_WHITE;
    moveCount = 4;
    gameOver = false;
    currentPlayer = PLAYER_BLACK;
}

bool isInBounds(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

bool checkDirection(int row, int col, int dr, int dc, int player) {
    int opponent = (player == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
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
    int opponent = (player == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
    int r = row + dr;
    int c = col + dc;
    
    while (isInBounds(r, c) && board[r][c] == opponent) {
        // Add to animation queue instead of immediately flipping
        if (animationCount < 64) {
            animRow[animationCount] = r;
            animCol[animationCount] = c;
            animFromPlayer[animationCount] = opponent;
            animToPlayer[animationCount] = player;
            animStartTime[animationCount] = gameTime;
            animProgress[animationCount] = 0.0f;
            animationCount++;
        }
        board[r][c] = player; // Still update board state
        r += dr;
        c += dc;
    }
}

void makeMove(int row, int col, int player) {
    board[row][col] = player;
    moveCount++;
    
    // Reset animations
    animationCount = 0;
    isAnimating = true;
    
    int directions[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    
    for (int i = 0; i < 8; i++) {
        if (checkDirection(row, col, directions[i][0], directions[i][1], player)) {
            flipDirection(row, col, directions[i][0], directions[i][1], player);
        }
    }
    
    // If no pieces to flip, no animation needed
    if (animationCount == 0) {
        isAnimating = false;
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
            if (board[i][j] == PLAYER_BLACK) blackCount++;
            else if (board[i][j] == PLAYER_WHITE) whiteCount++;
        }
    }
}

int evaluateBoard(int player) {
    int blackCount, whiteCount;
    countPieces(blackCount, whiteCount);
    
    int cornerWeight = 25;
    int edgeWeight = 5;
    int score = 0;
    
    if (player == PLAYER_BLACK) {
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
    
    return score;
}

int minimax(int depth, bool isMaximizing, int player, int alpha, int beta) {
    if (depth == 0 || moveCount == BOARD_SIZE * BOARD_SIZE) {
        return evaluateBoard(player);
    }
    
    int opponent = (player == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
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

void updateAnimations() {
    if (!isAnimating) return;
    
    bool anyActive = false;
    for (int i = 0; i < animationCount; i++) {
        float elapsed = gameTime - animStartTime[i];
        animProgress[i] = elapsed / ANIMATION_DURATION;
        
        if (animProgress[i] < 1.0f) {
            anyActive = true;
        }
    }
    
    if (!anyActive) {
        isAnimating = false;
        animationCount = 0;
    }
}

void drawPiece(int x, int y, int player, float scale) {
    if (player == PLAYER_BLACK) {
        DrawCircle(x + 2, y + 2, 28 * scale, (Color){0, 0, 0, 100}); // Shadow
        DrawCircleGradient(x, y, 28 * scale, (Color){60, 60, 60, 255}, BLACK);
        if (scale > 0.5f) {
            DrawCircle(x - 8, y - 8, 8 * scale, (Color){80, 80, 80, 180}); // Highlight
        }
    } else if (player == PLAYER_WHITE) {
        DrawCircle(x + 2, y + 2, 28 * scale, (Color){0, 0, 0, 80}); // Shadow
        DrawCircleGradient(x, y, 28 * scale, WHITE, (Color){220, 220, 220, 255});
        if (scale > 0.5f) {
            DrawCircle(x - 8, y - 8, 8 * scale, (Color){255, 255, 255, 200}); // Highlight
        }
    }
}

void drawBoard() {
    // Draw checkerboard pattern
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            int x = BOARD_OFFSET_X + j * CELL_SIZE;
            int y = BOARD_OFFSET_Y + i * CELL_SIZE;
            
            // Checkerboard pattern: alternate between light and dark green
            Color cellColor;
            if ((i + j) % 2 == 0) {
                cellColor = (Color){15, 100, 30, 255};  // Dark green
            } else {
                cellColor = (Color){25, 140, 45, 255};  // Light green
            }
            DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, cellColor);
            DrawRectangleLines(x, y, CELL_SIZE, CELL_SIZE, (Color){10, 80, 25, 255});
        }
    }
    
    // Draw pieces with 3D effect
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            int x = BOARD_OFFSET_X + j * CELL_SIZE + CELL_SIZE / 2;
            int y = BOARD_OFFSET_Y + i * CELL_SIZE + CELL_SIZE / 2;
            
            // Check if this piece is animating
            bool isThisAnimating = false;
            float currentAnimProgress = 0.0f;
            int fromPlayer = EMPTY;
            int toPlayer = EMPTY;
            
            for (int a = 0; a < animationCount; a++) {
                if (animRow[a] == i && animCol[a] == j && animProgress[a] < 1.0f) {
                    isThisAnimating = true;
                    currentAnimProgress = animProgress[a];
                    fromPlayer = animFromPlayer[a];
                    toPlayer = animToPlayer[a];
                    break;
                }
            }
            
            if (isThisAnimating) {
                // Flip animation: scale from 1 -> 0 -> 1, switch color at midpoint
                float scale;
                int currentPlayer;
                
                if (currentAnimProgress < 0.5f) {
                    // First half: shrink and show from color
                    scale = 1.0f - (currentAnimProgress * 2.0f);
                    currentPlayer = fromPlayer;
                } else {
                    // Second half: grow and show to color
                    scale = (currentAnimProgress - 0.5f) * 2.0f;
                    currentPlayer = toPlayer;
                }
                
                drawPiece(x, y, currentPlayer, scale);
            } else if (board[i][j] != EMPTY) {
                // Normal piece rendering
                drawPiece(x, y, board[i][j], 1.0f);
            }
        }
    }
}

void drawEndGameGUI() {
    int blackCount, whiteCount;
    countPieces(blackCount, whiteCount);
    
    // Semi-transparent overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
    
    // Main panel - centered
    int panelWidth = 500;
    int panelHeight = 430;
    int panelX = (SCREEN_WIDTH - panelWidth) / 2;
    int panelY = (SCREEN_HEIGHT - panelHeight) / 2;
    
    // Panel shadow
    DrawRectangle(panelX + 5, panelY + 5, panelWidth, panelHeight, (Color){0, 0, 0, 100});
    
    // Panel background with gradient effect
    DrawRectangleGradientV(panelX, panelY, panelWidth, panelHeight, 
                          (Color){40, 120, 50, 255}, (Color){20, 80, 30, 255});
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, (Color){100, 200, 110, 255});
    DrawRectangleLines(panelX + 2, panelY + 2, panelWidth - 4, panelHeight - 4, (Color){80, 180, 90, 255});
    
    // Title - centered
    const char* title = "GAME OVER";
    int titleFontSize = 50;
    int titleWidth = MeasureText(title, titleFontSize);
    int titleX = panelX + (panelWidth - titleWidth) / 2;
    DrawText(title, titleX + 2, panelY + 32, titleFontSize, (Color){0, 0, 0, 100}); // Shadow
    DrawText(title, titleX, panelY + 30, titleFontSize, WHITE);
    
    // Scores with pieces - better spacing
    int scoreStartY = panelY + 110;
    int scoreSpacing = 85;
    
    // Black score - centered
    int blackPieceX = panelX + 110;
    DrawCircleGradient(blackPieceX, scoreStartY, 30, (Color){60, 60, 60, 255}, BLACK);
    DrawCircle(blackPieceX - 8, scoreStartY - 8, 8, (Color){80, 80, 80, 180});
    
    string blackScoreText = "Black: " + to_string(blackCount);
    DrawText(blackScoreText.c_str(), blackPieceX + 55, scoreStartY - 18, 35, WHITE);
    
    // White score - centered
    int whitePieceX = panelX + 110;
    DrawCircleGradient(whitePieceX, scoreStartY + scoreSpacing, 30, WHITE, (Color){220, 220, 220, 255});
    DrawCircle(whitePieceX - 8, scoreStartY + scoreSpacing - 8, 8, (Color){255, 255, 255, 200});
    
    string whiteScoreText = "White: " + to_string(whiteCount);
    DrawText(whiteScoreText.c_str(), whitePieceX + 55, scoreStartY + scoreSpacing - 18, 35, WHITE);
    
    // Winner announcement - centered
    const char* result;
    Color resultColor;
    if (blackCount > whiteCount) {
        result = "YOU WIN!";
        resultColor = (Color){255, 215, 0, 255}; // Gold
    } else if (whiteCount > blackCount) {
        result = "AI WINS!";
        resultColor = (Color){200, 200, 200, 255}; // Silver
    } else {
        result = "IT'S A TIE!";
        resultColor = (Color){150, 150, 255, 255}; // Blue
    }
    
    int resultFontSize = 40;
    int resultWidth = MeasureText(result, resultFontSize);
    int resultX = panelX + (panelWidth - resultWidth) / 2;
    int resultY = scoreStartY + scoreSpacing * 2;
    DrawText(result, resultX + 2, resultY + 2, resultFontSize, (Color){0, 0, 0, 150}); // Shadow
    DrawText(result, resultX, resultY, resultFontSize, resultColor);
    
    // Buttons - centered at bottom
    int buttonWidth = 140;
    int buttonHeight = 50;
    int buttonSpacing = 20;
    int totalButtonWidth = buttonWidth * 2 + buttonSpacing;
    int buttonStartX = panelX + (panelWidth - totalButtonWidth) / 2;
    int buttonY = panelY + panelHeight - 70;
    
    Rectangle playButton = {(float)buttonStartX, (float)buttonY, (float)buttonWidth, (float)buttonHeight};
    Rectangle quitButton = {(float)(buttonStartX + buttonWidth + buttonSpacing), (float)buttonY, (float)buttonWidth, (float)buttonHeight};
    
    Vector2 mousePos = GetMousePosition();
    bool playHover = CheckCollisionPointRec(mousePos, playButton);
    bool quitHover = CheckCollisionPointRec(mousePos, quitButton);
    
    // Play button
    Color playColor = playHover ? (Color){60, 180, 70, 255} : (Color){40, 140, 50, 255};
    DrawRectangleRec(playButton, playColor);
    DrawRectangleLinesEx(playButton, 2, (Color){100, 220, 110, 255});
    int playTextWidth = MeasureText("PLAY AGAIN", 20);
    DrawText("PLAY AGAIN", playButton.x + (buttonWidth - playTextWidth) / 2, playButton.y + 15, 20, WHITE);
    
    // Quit button
    Color quitColor = quitHover ? (Color){200, 60, 60, 255} : (Color){160, 40, 40, 255};
    DrawRectangleRec(quitButton, quitColor);
    DrawRectangleLinesEx(quitButton, 2, (Color){220, 100, 100, 255});
    int quitTextWidth = MeasureText("QUIT", 20);
    DrawText("QUIT", quitButton.x + (buttonWidth - quitTextWidth) / 2, quitButton.y + 15, 20, WHITE);
    
    // Handle clicks
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (playHover) {
            initBoard();
        } else if (quitHover) {
            CloseWindow();
        }
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Reversi (Othello) - AI Game");
    SetTargetFPS(60);
    
    initBoard();
    
    int pendingPlayer = EMPTY; // Track who should move next after animations
    
    while (!WindowShouldClose()) {
        // Update game time
        gameTime += GetFrameTime();
        
        // Update animations
        bool wasAnimating = isAnimating;
        updateAnimations();
        
        // If animations just finished, switch to pending player
        if (wasAnimating && !isAnimating && pendingPlayer != EMPTY) {
            currentPlayer = pendingPlayer;
            pendingPlayer = EMPTY;
        }
        
        // Handle player moves (only if not animating)
        if (!gameOver && !isAnimating && currentPlayer == PLAYER_BLACK && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            int col = (mousePos.x - BOARD_OFFSET_X) / CELL_SIZE;
            int row = (mousePos.y - BOARD_OFFSET_Y) / CELL_SIZE;
            
            if (isValidMove(row, col, currentPlayer)) {
                makeMove(row, col, currentPlayer);
                // Schedule turn switch after animation completes
                if (isAnimating) {
                    pendingPlayer = PLAYER_WHITE;
                } else {
                    currentPlayer = PLAYER_WHITE;
                }
            }
        }
        
        // AI move (only if not animating)
        if (!gameOver && !isAnimating && currentPlayer == PLAYER_WHITE) {
            if (hasValidMoves(PLAYER_WHITE)) {
                int row, col;
                getAIMove(row, col, PLAYER_WHITE);
                makeMove(row, col, PLAYER_WHITE);
                // Schedule turn switch after animation completes
                if (isAnimating) {
                    pendingPlayer = PLAYER_BLACK;
                } else {
                    currentPlayer = PLAYER_BLACK;
                }
            } else if (!hasValidMoves(PLAYER_BLACK)) {
                gameOver = true;
            } else {
                currentPlayer = PLAYER_BLACK;
            }
        }
        
        // Check for game over
        if (!gameOver && !hasValidMoves(PLAYER_BLACK) && !hasValidMoves(PLAYER_WHITE)) {
            gameOver = true;
        }
        
        BeginDrawing();
        ClearBackground((Color){15, 60, 25, 255});
        
        // Draw title
        const char* title = "REVERSI (OTHELLO)";
        int titleWidth = MeasureText(title, 40);
        DrawText(title, (SCREEN_WIDTH - titleWidth) / 2 + 2, 22, 40, (Color){0, 0, 0, 100});
        DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, 20, 40, (Color){255, 215, 0, 255});
        
        // Draw score
        int blackCount, whiteCount;
        countPieces(blackCount, whiteCount);
        string scoreText = "Black: " + to_string(blackCount) + "  |  White: " + to_string(whiteCount);
        int scoreWidth = MeasureText(scoreText.c_str(), 25);
        DrawText(scoreText.c_str(), (SCREEN_WIDTH - scoreWidth) / 2, 80, 25, WHITE);
        
        // Current player
        if (!gameOver) {
            const char* turnText = (currentPlayer == PLAYER_BLACK) ? "Your Turn (BLACK)" : "AI is thinking...";
            int turnWidth = MeasureText(turnText, 20);
            DrawText(turnText, (SCREEN_WIDTH - turnWidth) / 2, 115, 20, YELLOW);
        }
        
        drawBoard();
        
        if (gameOver) {
            drawEndGameGUI();
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
