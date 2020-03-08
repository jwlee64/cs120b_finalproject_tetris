#ifndef BOARD_H
#define BOARD_H

#include "io.h"

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

unsigned char board[64];

// board funcs vvv =============================================================================================
// creates custom char at location loc with info p
void LCDBuildChar(unsigned char loc, unsigned char *p){
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

void resetBoard() {
	for (int i = 0; i < 64; i++){
		board[i] = defaultBoard[i];
	}
}
// board funcs ^^^ =============================================================================================
#endif //BOARD_H