#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include <math.h>

typedef struct gameState {
	int** grid;
	long score;
	int blockct;
} Game;

int main();
void draw(Game* game);
void drawGrid(int** grid);
int calcNumDigits(int num);
void initColors();
int calcColor(int num);
int doTick(Game* game, int direction);
void newBlock(int** grid, int* blockctptr);
void shift(int** grid, int* blockctptr, long* score, int direction);
void gameLoss(Game* game);
int quit(Game* game);