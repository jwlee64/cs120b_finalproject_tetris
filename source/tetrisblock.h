#ifndef TETRISBLOCK_H
#define TETRISBLOCK_H

#include "block.h"

typedef struct _tetrisBlock{
	unsigned char type;
	int rot;
	block *b1;
	block *b2;
	block *b3;
	block *b4;
} tetrisBlock;

// tetris block funcs vvv =============================================================================================
// sets pixels of all blocks to 1
void setTetrisBlock ( tetrisBlock *tb ) {
	setBlock(tb->b4);
	setBlock(tb->b3);
	setBlock(tb->b2);
	setBlock(tb->b1);
}
// sets pixels of all blocks to 0
void clearTetrisBlock ( tetrisBlock *tb ) {
	clearBlock(tb->b4);
	clearBlock(tb->b3);
	clearBlock(tb->b2);
	clearBlock(tb->b1);
}
// returns true if pixels below all blocks in tetris block is empty
bool canMoveTetrisBlockDown ( tetrisBlock *tb ){
	return (canMoveBlockDown(tb->b1) && canMoveBlockDown(tb->b2) && canMoveBlockDown(tb->b3) && canMoveBlockDown(tb->b4));
}
// returns true if pixels to the left of all blocks in tetris block is empty
bool canMoveTetrisBlockLeft ( tetrisBlock *tb ){
	return (canMoveBlockLeft(tb->b1) && canMoveBlockLeft(tb->b2) && canMoveBlockLeft(tb->b3) && canMoveBlockLeft(tb->b4));
}
// returns true if pixels to the right of all blocks in tetris block is empty
bool canMoveTetrisBlockRight ( tetrisBlock *tb ){
	return (canMoveBlockRight(tb->b1) && canMoveBlockRight(tb->b2) && canMoveBlockRight(tb->b3) && canMoveBlockRight(tb->b4));
}
// sets pixels below all blocks to 1
bool moveTetrisBlockDown ( tetrisBlock *tb ){
	clearTetrisBlock(tb);
	if (!canMoveTetrisBlockDown(tb)){
		setTetrisBlock(tb);
		return false;
	}
	moveBlockDown(tb->b4);
	moveBlockDown(tb->b3);
	moveBlockDown(tb->b2);
	moveBlockDown(tb->b1);
	return true;
}
// sets pixels to the left of all blocks to 1
bool moveTetrisBlockLeft ( tetrisBlock *tb ){
	clearTetrisBlock(tb);
	if (!canMoveTetrisBlockLeft(tb)){
		setTetrisBlock(tb);
		return false;
	}
	moveBlockLeft(tb->b4);
	moveBlockLeft(tb->b3);
	moveBlockLeft(tb->b2);
	moveBlockLeft(tb->b1);
	return true;
}
// sets pixels to the right of all blocks to 1
bool moveTetrisBlockRight ( tetrisBlock *tb ){
	clearTetrisBlock(tb);
	if (!canMoveTetrisBlockRight(tb)){
		setTetrisBlock(tb);
		return false;
	}
	moveBlockRight(tb->b4);
	moveBlockRight(tb->b3);
	moveBlockRight(tb->b2);
	moveBlockRight(tb->b1);
	return true;
}

// returns pointer to a new tetris block of type b
tetrisBlock* createTetrisBlock (int b) {
	static tetrisBlock tb;
	static block b1, b2, b3, b4;
	tb.b1 = &b1; tb.b2 = &b2;
	tb.b3 = &b3; tb.b4 = &b4;
	tb.rot = 0;

	// most common values for each block
	b1.row = 31; b1.bit = 1; 
	b2.row = 27; b2.bit = 1;
	b3.row = 35; b3.bit = 1;
	b4.row = 31; b4.bit = 2;

	switch (b) {
		case 0: // J
			tb.type = 'J';
			b4.row = 35;
		break;
		case 1: // L
			tb.type = 'L';
			b4.row = 27;
		break;
		case 2: // O
			tb.type = 'O';
			b3.row = 27;
			b3.bit = 2;	
		break;
		case 3: // S
			tb.type = 'S';
			b2.bit = 2;
		break;
		case 4: // T 
			tb.type = 'T';
		break;
		case 5: // Z
			tb.type = 'Z';
			b3.bit = 2;
		break;
		default: // I
			tb.type = 'I';
			b4.row = 23;
			b4.bit = 1;
		break;
	}
	setTetrisBlock(&tb);
	return &tb;
}

// rotates blocks of tetris block based of current rotation and type
void rotateTetrisBlock(tetrisBlock *tb) {
	clearTetrisBlock(tb);
	tb->rot += 1;
	switch(tb->type) {
		case 'J':
			switch ( (tb->rot)%4 ){
				case 0:
					if ( ! ( canMoveBlock(tb->b2, 1, -1) && canMoveBlock(tb->b3, -1, 1) && canMoveBlock(tb->b4, -2, 0) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, -1);
					moveBlock(tb->b3, -1, 1);
					moveBlock(tb->b4, -2, 0);
				break;
				case 1:
					if ( ! ( canMoveBlock(tb->b2, 1, 1) && canMoveBlock(tb->b3, -1, -1) && canMoveBlock(tb->b4, 0, -2) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, 1);
					moveBlock(tb->b3, -1, -1);
					moveBlock(tb->b4, 0, -2);
				break;
				case 2:
					if ( ! ( canMoveBlock(tb->b2, -1, 1) && canMoveBlock(tb->b3, 1, -1) && canMoveBlock(tb->b4, 2, 0) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, 1);
					moveBlock(tb->b3, 1, -1);
					moveBlock(tb->b4, 2, 0);
				break;
				case 3:
					if ( ! ( canMoveBlock(tb->b2, -1, -1) && canMoveBlock(tb->b3, 1, 1) && canMoveBlock(tb->b4, 0, 2) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, -1);
					moveBlock(tb->b3, 1, 1);
					moveBlock(tb->b4, 0, 2);
				break;
				default: break;
			}
		break;
		case 'L':
			switch ( (tb->rot)%4 ){
				case 0:
					if ( ! ( canMoveBlock(tb->b2, -1, -1) && canMoveBlock(tb->b3, -1, 1) && canMoveBlock(tb->b4, 0, 2) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, -1);
					moveBlock(tb->b3, -1, 1);
					moveBlock(tb->b4, 0, -2);
				break;
				case 1:
					if ( ! ( canMoveBlock(tb->b2, 1, 1) && canMoveBlock(tb->b3, -1, -1) && canMoveBlock(tb->b4, 2, 0) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, 1);
					moveBlock(tb->b3, -1, -1);
					moveBlock(tb->b4, 2, 0);
				break;
				case 2:
					if ( ! ( canMoveBlock(tb->b2, -1, 1) && canMoveBlock(tb->b3, 1, -1) && canMoveBlock(tb->b4, 0, 2) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, 1);
					moveBlock(tb->b3, 1, -1);
					moveBlock(tb->b4, 0, 2);
				break;
				case 3:
					if ( ! ( canMoveBlock(tb->b2, -1, -1) && canMoveBlock(tb->b3, 1, 1) && canMoveBlock(tb->b4, -2, 0) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, -1);
					moveBlock(tb->b3, 1, 1);
					moveBlock(tb->b4, -2, 0);
				break;
				default: break;
			}
		break;
		case 'O': break;
		case 'S':
			switch ( (tb->rot)%4 ){
				case 0:
					if ( ! ( canMoveBlock(tb->b2, 0, -2) && canMoveBlock(tb->b3, -1, 1) && canMoveBlock(tb->b4, -1, -1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 0, -2);
					moveBlock(tb->b3, -1, 1);
					moveBlock(tb->b4, -1, -1);
				break;
				case 1:
					if ( ! ( canMoveBlock(tb->b2, 2, 0) && canMoveBlock(tb->b3, -1, -1) && canMoveBlock(tb->b4, 1, -1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 2, 0);
					moveBlock(tb->b3, -1, -1);
					moveBlock(tb->b4, 1, -1);
				break;
				case 2:
					if ( ! ( canMoveBlock(tb->b2, 0, 2) && canMoveBlock(tb->b3, 1, -1) && canMoveBlock(tb->b4, 1, 1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 0, 2);
					moveBlock(tb->b3, 1, -1);
					moveBlock(tb->b4, 1, 1);
				break;
				case 3:
					if ( ! ( canMoveBlock(tb->b2, -2, 0) && canMoveBlock(tb->b3, 1, 1) && canMoveBlock(tb->b4, -1, 1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -2, 0);
					moveBlock(tb->b3, 1, 1);
					moveBlock(tb->b4, -1, 1);
				break;
				default: break;
			}
		break;
		case 'T':
			switch ( (tb->rot)%4 ){
				case 0:
					if ( ! ( canMoveBlock(tb->b2, 1, -1) && canMoveBlock(tb->b3, -1, 1) && canMoveBlock(tb->b4, -1, -1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, -1);
					moveBlock(tb->b3, -1, 1);
					moveBlock(tb->b4, -1, -1);
				break;
				case 1:
					if ( ! ( canMoveBlock(tb->b2, 1, 1) && canMoveBlock(tb->b3, -1, -1) && canMoveBlock(tb->b4, 1, -1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, 1);
					moveBlock(tb->b3, -1, -1);
					moveBlock(tb->b4, 1, -1);
				break;
				case 2:
					if ( ! ( canMoveBlock(tb->b2, -1, 1) && canMoveBlock(tb->b3, 1, -1) && canMoveBlock(tb->b4, 1, 1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, 1);
					moveBlock(tb->b3, 1, -1);
					moveBlock(tb->b4, 1, 1);
				break;
				case 3:
					if ( ! ( canMoveBlock(tb->b2, -1, -1) && canMoveBlock(tb->b3, 1, 1) && canMoveBlock(tb->b4, -1, 1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, -1);
					moveBlock(tb->b3, 1, 1);
					moveBlock(tb->b4, -1, 1);
				break;
				default: break;
			}
		break;
		case 'Z':
			switch ( (tb->rot)%4 ){
				case 0:
					if ( ! ( canMoveBlock(tb->b2, 1, -1) && canMoveBlock(tb->b3, -2, 0) && canMoveBlock(tb->b4, -1, -1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, -1);
					moveBlock(tb->b3, -2, 0);
					moveBlock(tb->b4, -1, -1);
				break;
				case 1:
					if ( ! ( canMoveBlock(tb->b2, 1, 1) && canMoveBlock(tb->b3, 0, -2) && canMoveBlock(tb->b4, 1, -1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, 1);
					moveBlock(tb->b3, 0, -2);
					moveBlock(tb->b4, 1, -1);
				break;
				case 2:
					if ( ! ( canMoveBlock(tb->b2, -1, 1) && canMoveBlock(tb->b3, 2, 0) && canMoveBlock(tb->b4, 1, 1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, 1);
					moveBlock(tb->b3, 2, 0);
					moveBlock(tb->b4, 1, 1);
				break;
				case 3:
					if ( ! ( canMoveBlock(tb->b2, -1, -1) && canMoveBlock(tb->b3, 0, 2) && canMoveBlock(tb->b4, -1, 1) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, -1);
					moveBlock(tb->b3, 0, 2);
					moveBlock(tb->b4, -1, 1);
				break;
				default: break;
			}
		break;
		case 'I':
			switch ( (tb->rot)%4 ){
				case 0:
					if ( ! ( canMoveBlock(tb->b2, 1, 1) && canMoveBlock(tb->b3, -1, 1) && canMoveBlock(tb->b4, 2, -2) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, -1);
					moveBlock(tb->b3, -1, 1);
					moveBlock(tb->b4, 2, -2);
				break;
				case 1:
					if ( ! ( canMoveBlock(tb->b2, 1, 1) && canMoveBlock(tb->b3, -1, -1) && canMoveBlock(tb->b4, 2, 2) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, 1, 1);
					moveBlock(tb->b3, -1, -1);
					moveBlock(tb->b4, 2, 2);
				break;
				case 2:
					if ( ! ( canMoveBlock(tb->b2, -1, 1) && canMoveBlock(tb->b3, 1, -1) && canMoveBlock(tb->b4, -2, 2) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, 1);
					moveBlock(tb->b3, 1, -1);
					moveBlock(tb->b4, -2, 2);
				break;
				case 3:
					if ( ! ( canMoveBlock(tb->b2, -1, -1) && canMoveBlock(tb->b3, 1, 1) && canMoveBlock(tb->b4, -2, -2) ) ){
						tb->rot -= 1;
						break;
					}
					moveBlock(tb->b2, -1, -1);
					moveBlock(tb->b3, 1, 1);
					moveBlock(tb->b4, -2, -2);
				break;
				default: break;
			}
		break;
		default: break;
	}
	setTetrisBlock(tb);
}
// tetris block funcs ^^^ =============================================================================================
#endif //TETRISBLOCK_H