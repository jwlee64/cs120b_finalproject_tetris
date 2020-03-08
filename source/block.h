#ifndef BLOCK_H
#define BLOCK_H

#include "board.h"

typedef struct _block{
	int row; // 31 for center
	int bit; // 1 for topmost bit
} block;

// pixel funcs vvv =============================================================================================
// set a pix to 0
void clearPix( int row, int bit) {
	board[row] = board[row] & ~((int)( 1 << bit));
}
// set a pix to 1
void setPix( int row, int bit) {
	board[row] = board[row] | ( 1 << bit);
}
// sets the pix above passed info to 1
void movePixUp ( int row, int bit ){
	if (bit == 0)
		setPix( row+1 , 4);
	else 
		setPix( row , bit - 1);
}
// sets the pix below passed info to 1
void movePixDown ( int row, int bit ){
	if (bit == 4)
		setPix( row-1 , 0);
	else 
		setPix( row, bit + 1);
}
// sets the pix to the left of passed info to 1
void movePixLeft ( int row, int bit ){
	setPix( row-4 , bit);
}
// sets the pix to the right of passed info to 1
void movePixRight ( int row, int bit ){
	setPix( row+4 , bit);
}
// returns true if bit is 1 false if 0
bool checkBit ( int row, int bit ) {
	return (board[row] & (1 << bit));
}
//
unsigned char getBit ( int row, int bit ) {
	if (checkBit (row, bit))
		return 0b1;
	return 0b0;
}
// pixel funcs ^^^ =============================================================================================

// block funcs vvv =============================================================================================
// sets pix of block to 1
void setBlock ( block *b ) {
	setPix( b->row, b->bit );
}
// sets pix of block to 0
void clearBlock ( block *b ) {
	clearPix( b->row, b->bit );
}
// returns true if pix below block is empty
bool canMoveBlockDown ( block *b ){
	if (b->bit == 4){
		if (checkBit(b->row-1, 0))
			return false;
	}
	else if (checkBit(b->row ,(b->bit)+1))
			return false;
	return true;
}
// returns true if pix to the left of block is empty
bool canMoveBlockLeft ( block *b ){
	if (checkBit(b->row-4, b->bit))
		return false;
	return true;
}
// returns true if pix to the right of block is empty
bool canMoveBlockRight ( block *b ){
	if (checkBit(b->row+4, b->bit))
		return false;
	return true;
}
// general can move block function
// returns true if pix up/down v and left/right h from block is empty
bool canMoveBlock ( block *b , signed int v, signed int h){
	if ( (((h*4) + b->row) > 58) || (((h*4) + b->row) < 4) ){ // moving horizontal outside of the boundaries
		return false;
	}

	if ( ((((h*4) + b->row) % 4 ) == 0)  && ((b->bit - v) > 3)){ // check down and lowest row
		return false;
	}
	else if ( (b->bit - v) > 4){ // down check
		return !checkBit( (h*4) + b->row - 1 , (b->bit - v) - 5);
	}
	else if ( ((((h*4) + b->row) % 4) == 3) && ((b->bit - v) < 1 )){ // check down and highest row
		return false;
	}
	else if ( (b->bit - v) < 0){ // up check
		return !checkBit( (h*4) + b->row + 1 , (b->bit - v )+ 5);
	}
	else {
		return !checkBit( (h*4) + b->row, b->bit - v);
	}
	return true;
}
// sets pix below block to 1
void moveBlockDown ( block *b ){
	movePixDown( b->row, b->bit );
	if (b->bit == 4){
		b->row = b->row-1;
		b->bit = 0;
	}
	else 
		b->bit = b->bit+1;
}
// sets pix above block to 1
void moveBlockUp ( block *b ){
	movePixUp( b->row, b->bit );
	if (b->bit == 0){
		b->row = b->row+1;
		b->bit = 4;
	}
	else 
		b->bit = b->bit-1;
}
// sets pix to the left of block to 1
void moveBlockLeft ( block *b ){
	movePixLeft( b->row, b->bit );
	b->row = b->row-4;
}
// sets pix to the right of block to 1
void moveBlockRight ( block *b ){
	movePixRight( b->row, b->bit );
	b->row = b->row+4;
}
// general move block function
// returns sets pix up/down v and left/right h from block to 1
void moveBlock ( block *b , int v, int h){
	if ( v < 0 ){
		for ( int i = 0; i < abs(v); i++){
			clearBlock(b);
			moveBlockDown(b);
		}
	}
	else if (v > 0){
		for ( int i = 0; i < abs(v); i++){
			clearBlock(b);
			moveBlockUp(b);
		}
	}
	if ( h < 0 ){
		for ( int i = 0; i < abs(h); i++){
			clearBlock(b);
			moveBlockLeft(b);
		}
	}
	else if (h > 0){
		for ( int i = 0; i < abs(h); i++){
			clearBlock(b);
			moveBlockRight(b);
		}
	}
}
// block funcs ^^^ =============================================================================================
#endif //BLOCK_H