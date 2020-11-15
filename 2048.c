#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include "2048.h"

#define GRID_SIZE 4
#define GRID_AREA 16
#define UP 1
#define DOWN 2
#define RIGHT 3
#define LEFT 4

struct termios oldt, newt; // terminal settings

/*
 * Main function containing game loop.
 * @r exit code
 */
int main() {
	initscr();
	noecho();

	/*
 	 * Initialize game and necessary variables.
 	 */
	Game* game = malloc(sizeof(Game));
	game->grid = malloc(sizeof(int*) * GRID_SIZE);
	for(int i = 0; i < GRID_SIZE; ++i) {
		game->grid[i] = malloc(sizeof(int) * 4);
		for(int j = 0; j < GRID_SIZE; ++j) {
			game->grid[i][j] = 0;
		}
	}
	game->blockct = 0;
	game->score = 0;
	srand(time(0));							// Set RNG
	newBlock(game->grid, &(game->blockct));	// The game requires 
	newBlock(game->grid, &(game->blockct));	// 2 blocks to begin with

	draw(game);
	/*
	 * Game loop.
	 */
	char c;
	int exitFlag;
	while(1) {
		c = getchar();
		switch(c) {
			case 'w':
				exitFlag = doTick(game, UP);
				break;
			case 's':
				exitFlag = doTick(game, DOWN);
				break;
			case 'a':
				exitFlag = doTick(game, LEFT);
				break;
			case 'd':
				exitFlag = doTick(game, RIGHT);
				break;
			case 'q' :
				printw("Are you sure you want to quit? [y/N]\n");
				refresh();
				c = getchar();
				if(c == 'y' || c == 'Y') {
					return quit(game);
				}
				draw(game);
				break;
			default :
				draw(game);
				printw("Invalid direction. Use WASD for directions.\n");
				refresh();
				break;
		}
		if(exitFlag != -1) {
			return quit(game);
		}
	}
}

/*
 * Draws the game screen.
 * @p game - the current gamestate
 */
void draw(Game* game) {
	clear();
	printw("Score: %ld\n", game->score);
	drawGrid(game->grid);
}

/*
 * Prints a GRID_SIZE x GRID_SIZE int** grid to stdout.
 * @p grid - a square 2D array of integers
 * @r void
 */
void drawGrid(int** grid) {
	int val;
	int whitespaceInCol[GRID_SIZE];
	for(int z = 0; z < GRID_SIZE; ++z) {
		whitespaceInCol[z] = 1;
	}
	for(int i = 0; i < GRID_SIZE; ++i) {
		for(int j = 0; j < GRID_SIZE; ++j) {
			for(int k = 0; k < GRID_SIZE; ++k) {
				int numDigits = calcNumDigits(grid[k][j]);
				if(whitespaceInCol[j] < numDigits) {
					whitespaceInCol[j] = numDigits;
				}
			}
			val = grid[i][j];
			int numDigits = calcNumDigits(grid[i][j]);

			printw("[ ");
			for(int k = 0; k < (whitespaceInCol[j] - numDigits); ++k) {
					printw(" ");
				}
			if(val != 0) {
				printw("%i", grid[i][j]);
			}
			printw(" ]");
		}
		printw("\n");
		refresh();
	}
}

/*
 * Returns the number of digits in num.
 * @p num - an integer from values[]
 * @r the number of digits in num if num > 0. Else, return 0
 */
int calcNumDigits(int num) {
	if(num <= 0) {
		return 0;
	}
	else if(num % 1024 == 0) {
		return 4;
	} 
	else if(num % 128 == 0) {
		return 3;
	}
	else if(num %16 == 0) {
		return 2;
	}
	return 1;
}

/*
 * Do a game tick.
 * @p game - the game whose state is being updated
 * @p direction - direction parameter to be passed to shift()
 * @r a flag which tells the main function whether to quit
 */
int doTick(Game* game, int direction) {
	if(game->blockct >= 16) {
		gameLoss(game);
		return 0;
	}
	shift(game->grid, &(game->blockct), &(game->score), direction);
	newBlock(game->grid, &(game->blockct));
	draw(game);
	//Sprintw("Game was ticked, %i\n", game->blockct);
	return -1;
}

/*
 * Inserts a number into the grid at a random empty location.
 * There is a 10% chance the number will be a 4,
 * and a 90% chance the number will be a 2.
 * @p grid - the grid that the number will be inserted into
 * @p blockct - the number of blocks in the grid
 */
void newBlock(int** grid, int* blockctptr) {
	int loc = rand() % (GRID_AREA - *blockctptr);
	int tilechance = ((rand() % 10) == 0);
	int idx = 0;
	for(int i = 0; i < GRID_SIZE; ++i) {
		for(int j = 0; j < GRID_SIZE; ++j) {
			if(grid[i][j]) {
				continue;
			}
			else if(idx == loc){
				grid[i][j] = 4 * tilechance + 2 * (1 - tilechance);
				*blockctptr += 1;
				return;
			}
			else {
				idx++;
			}
		}
	}
}

/*
 * Perform a shift as defined by the rules of 2048 in the specified
 * direction.
 * @p grid - the grid the shift will be performed on
 * @p direction - the direction of the shift
 */
void shift(int** grid, int* blockctptr, long* score, int direction) {
	int merged[GRID_SIZE][GRID_SIZE] = {0};
	if(direction == UP) {
		for(int c = 0; c < GRID_SIZE; ++c) {
			for(int r = 1; r < GRID_SIZE; ++r) {
				int idx = r;
				while(idx > 0) {
					if(grid[idx][c] == 0) {
						//
					}
					else if(merged[idx-1][c] != 1 && merged[idx][c] != 1 &&
					 grid[idx][c] == grid[idx-1][c]) {
						grid[idx-1][c] = grid[idx][c] + grid[idx-1][c];
						*score += grid[idx-1][c];
						grid[idx][c] = 0;
						merged[idx-1][c] = 1;
						*blockctptr -= 1;
					}
					else if(grid[idx-1][c] == 0) {
						grid[idx-1][c] = grid[idx][c];
						grid[idx][c] = 0;
						merged[idx-1][c] = merged[idx][c];
						merged[idx][c] = 0;
					}
					--idx;
				}
			}
		}
	}
	else if(direction == DOWN) {
		for(int c = 0; c < GRID_SIZE; ++c) {
			for(int r = GRID_SIZE-2; r >= 0; --r) {
				int idx = r;
				while(idx < GRID_SIZE-1) {
					if(grid[idx][c] == 0) {
						//
					}
					else if(merged[idx+1][c] != 1 && merged[idx][c] != 1 &&
					 grid[idx][c] == grid[idx+1][c]) {
						grid[idx+1][c] = grid[idx][c] + grid[idx+1][c];
						*score += grid[idx+1][c];
						grid[idx][c] = 0;
						merged[idx+1][c] = 1;
						*blockctptr -= 1;
					}
					else if(grid[idx+1][c] == 0) {
						grid[idx+1][c] = grid[idx][c];
						grid[idx][c] = 0;
						merged[idx+1][c] = merged[idx][c];
						merged[idx][c] = 0;
					}
					++idx;
				}
			}
		}
	}
	else if(direction == LEFT) {
		for(int r = 0; r < GRID_SIZE; ++r) {
			for(int c = 1; c < GRID_SIZE; ++c) {
				int idx = c;
				while(idx > 0) {
					if(grid[r][idx] == 0) {
						//
					}
					else if(merged[r][idx-1] != 1 && merged[r][idx] != 1&&
					 grid[r][idx] == grid[r][idx-1]) {
						grid[r][idx-1] = grid[r][idx] + grid[r][idx-1];
						*score += grid[r][idx-1];
						grid[r][idx] = 0;
						merged[r][idx-1] = 1;
						*blockctptr -= 1;
					}
					else if(grid[r][idx-1] == 0) {
						grid[r][idx-1] = grid[r][idx];
						grid[r][idx] = 0;
						merged[r][idx-1] = merged[r][idx];
						merged[r][idx] = 0;
					}
					--idx;
				}
			}
		}
	}
	else {
		for(int r = 0; r < GRID_SIZE; ++r) {
			for(int c = GRID_SIZE-2; c >= 0; --c) {
				int idx = c;
				while(idx < GRID_SIZE-1) {
					if(grid[r][idx] == 0) {
						//
					}
					else if(merged[r][idx+1] != 1 && merged[r][idx] != 1 &&
					 grid[r][idx] == grid[r][idx+1]) {
						grid[r][idx+1] = grid[r][idx+1] + grid[r][idx+1];
						*score += grid[r][idx+1];
						grid[r][idx] = 0;
						merged[r][idx+1] = 1;
						*blockctptr -= 1;
					}
					else if(grid[r][idx+1] == 0) {
						grid[r][idx+1] = grid[r][idx];
						grid[r][idx] = 0;
						merged[r][idx+1] = merged[r][idx];
						merged[r][idx] = 0;
						
					}
					++idx;
				}
			}
		}
	}
}

/*
 * Game loss state.
 */
void gameLoss(Game* game) {
	draw(game);
	printw("Game over. Final Score: %ld\nPress any key to quit.\n", game->score);
	refresh();
	getchar();
}

/*
 * Restore saved terminal settings and exit.
 * @r 0 (If the program calls this function, operation was successful)
 */
int quit(Game* game) {
	endwin();
	
	for(int i = 0; i < GRID_SIZE; ++i) {
		free(game->grid[i]);
	}
	free(game->grid);
	free(game);
	return 0;
}