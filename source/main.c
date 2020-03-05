/*	Author: josiahlee
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Final Project
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/eeprom.h>

#include "io.h"
#include "io.c"
#include "timer.h"
#include "keypad.h"
#include "scheduler.h"

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

// Structs vvv =============================================================================================
typedef struct _block{
	int row; // 31 for center
	int bit; // 2 for topmost bit
} block;

typedef struct _tetrisBlock{
	unsigned char type;
	int rot;
	block *b1;
	block *b2;
	block *b3;
	block *b4;
} tetrisBlock;

// Structs ^^^ =============================================================================================

// ADC vvv =============================================================================================
// ADC Initialization 
void ADC_init() {
    ADMUX|=(1<<REFS0);   
    ADCSRA|=(1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); //ENABLE ADC, PRESCALER 128
}

// read from an adc register
uint16_t ADC_read(uint8_t ch){
    ch&=0b00000111;         		//ANDing to limit input to 7
    ADMUX = (ADMUX & 0xf8)|ch; 		//Clear last 3 bits of ADMUX, OR with ch
    ADCSRA|=(1<<ADSC);        		//START CONVERSION
    while((ADCSRA)&(1<<ADSC));    	//WAIT UNTIL CONVERSION IS COMPLETE
    return(ADC);        			//RETURN ADC VALUE
}
// ADC ^^^ =============================================================================================


// global vars vvv =============================================================================================
const unsigned char defaultBoard[64] = {
	0b11111,	0b11111,	0b11111, 	0b11111,		
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,	
	0b10000,	0b00000,	0b00000,	0b00001,

	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b10000,	0b00000,	0b00000,	0b00001,
	0b11111,	0b11111,	0b11111,	0b11111,	
};

unsigned char board[64]; // board
tetrisBlock *tb; // current block
// global vars ^^^ =============================================================================================

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
void moveTetrisBlockDown ( tetrisBlock *tb ){
	clearTetrisBlock(tb);
	if (!canMoveTetrisBlockDown(tb)){
		setTetrisBlock(tb);
		return;
	}
	moveBlockDown(tb->b4);
	moveBlockDown(tb->b3);
	moveBlockDown(tb->b2);
	moveBlockDown(tb->b1);
}
// sets pixels to the left of all blocks to 1
void moveTetrisBlockLeft ( tetrisBlock *tb ){
	clearTetrisBlock(tb);
	if (!canMoveTetrisBlockLeft(tb)){
		setTetrisBlock(tb);
		return;
	}
	moveBlockLeft(tb->b4);
	moveBlockLeft(tb->b3);
	moveBlockLeft(tb->b2);
	moveBlockLeft(tb->b1);
}
// sets pixels to the right of all blocks to 1
void moveTetrisBlockRight ( tetrisBlock *tb ){
	clearTetrisBlock(tb);
	if (!canMoveTetrisBlockRight(tb)){
		setTetrisBlock(tb);
		return;
	}
	moveBlockRight(tb->b4);
	moveBlockRight(tb->b3);
	moveBlockRight(tb->b2);
	moveBlockRight(tb->b1);
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
	LCD_Cursor(25);
	LCD_WriteData('0' + tb->rot);
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

// creates custom char at location loc with info p
void LCDBuildChar(unsigned char loc, unsigned char *p)
{
 unsigned char i;
 if(loc<8) //If valid address
 {
  LCD_WriteCommand(0x40+(loc*8)); //Write to CGRAM
  for(i=0;i<8;i++)
  LCD_WriteData(p[i]); //Write the character pattern to CGRAM
 }
  LCD_WriteCommand(0x80); //shift back to DDRAM location 0
}
// creates custom char at location loc from board section place
void buildBoardChar( unsigned char loc , unsigned char place) {
	unsigned char temp[8]; // initialize temp to 8 vals of board
	if (loc > 3){
		for (unsigned char i = 0; i < 8; i++){
			temp[i] = board[31 + loc-3 + (i*4)];
		}
	}
	else if (loc < 4){
		for (unsigned char i = 0; i < 8; i++){
			temp[i] = board[loc + (i*4)];
		}
	}
	LCDBuildChar(place, temp); // build char
}
// creates custom chars for 8 board sections and writes to LCD
void displayBoard() {
	for ( unsigned char i = 0; i < 8; i++ ){
		buildBoardChar(i,i);
		if (i > 3){
			LCD_Cursor(13 + i);
		}
		else {
			LCD_Cursor(1+ i);
		}
		LCD_WriteData(i);
	}
}


signed char x;
unsigned char key;

int keyPadTick( int state ) {
	switch (state) {
		case 0: 
			return 1; break;
		case 1: 
			key = GetKeypadKey();
			if ( key != '\0'){
				if ( key == '5')
					moveTetrisBlockDown(tb);
				else if (key == '8')
					moveTetrisBlockLeft(tb);
				else if (key == '2')
					moveTetrisBlockRight(tb);
				else if (key == '4')
					rotateTetrisBlock(tb);
				else if (key == '1'){
				 	clearTetrisBlock(tb);
				 	x++;
					tb = createTetrisBlock(x);				
				}
				return 2;
			}
			return 1;
			break;
		case 2: 
			key = GetKeypadKey();
			if ( key == '\0'){
				return 1;
			}
			return 2;
			break;
		default:
			return 0;
	}
	return 0;
}

unsigned char tetrisCnt;

int tetrisTick( int state ) {
	switch (state) {
		case 0: 
			tetrisCnt = 0;
			return 1; break;
		case 1: 	
			if (tetrisCnt < 20){
				tetrisCnt++;
			}
			else {
				tetrisCnt = 0;
				moveTetrisBlockDown(tb);
			}
			displayBoard();
			return 1;
		break;
		default:
			return 0;
	}
	return 0;
}

const unsigned short lrMax = 1023;
const unsigned short udMax = 1023;

unsigned char joystickCnt;

int joystickTick( int state ) {
	switch (state) {
		case 0: 
			joystickCnt = 0;
			return 1; break;
		case 1: 	
			LCD_Cursor(12);
			if (ADC_read(0) > lrMax*3/4){
				LCD_WriteData('R');
				moveTetrisBlockRight(tb);
			}
			else if (ADC_read(0) < lrMax/4){
				LCD_WriteData('L');
				moveTetrisBlockLeft(tb);
			}
			else
				LCD_WriteData('0');
			
			if (ADC_read(1) > udMax*3/4){
				LCD_WriteData('U');
				rotateTetrisBlock(tb);
			}
			else if (ADC_read(1) < udMax/4) {
				LCD_WriteData('D');
				moveTetrisBlockDown(tb);
			}
			else
				LCD_WriteData('0');

			joystickCnt = 0;
			return 2;
			break;
		case 2: // delay 100ms after reading once
			if (joystickCnt <10){
				joystickCnt++;
				return 2;
			}
			else{
				joystickCnt = 0;
				return 1;
			}
			break;
		default:
			return 0;
	}
	return 0;
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xF0; PORTB = 0x0F; // Keypad Line
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines

	board = defaultBoard;

 	static task task1, task2, task3;
    task *tasks[] = { &task1, &task2, &task3 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    task1.state = 0;
    task1.period = 50;
    task1.elapsedTime = task1.period;
    task1.TickFct  = &keyPadTick;

    task2.state = 0;
    task2.period = 50;
    task2.elapsedTime = task2.period;
    task2.TickFct  = &tetrisTick;

    task3.state = 0;
    task3.period = 10;
    task3.elapsedTime = task3.period;
    task3.TickFct  = &joystickTick;

    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for ( i = 1 ; i < numTasks ; i++ ) {
    	GCD = findGCD(GCD, tasks[i]->period);
    }

    x = -1;
    // uint8_t ByteOfData;
    // eeprom_write_byte ((uint8_t*)46 , '1');
	// ByteOfData = eeprom_read_byte((uint8_t*)46);
 	// LCD_WriteData(ByteOfData);
	LCD_init();
	ADC_init();
    TimerSet(GCD);
    TimerOn();

    while (1) {
    	for ( i = 0 ; i < numTasks ; i++ ) {
    		if ( tasks[i]->elapsedTime == tasks[i]->period ) {
    			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
    			tasks[i]->elapsedTime = 0;
    		}
    		tasks[i]->elapsedTime += GCD;
    	}
    	while(!TimerFlag);
    	TimerFlag = 0;
    }
    return 0;
}
