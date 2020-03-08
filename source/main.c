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

#include "board.h"
#include "block.h"
#include "tetrisBlock.h"

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

const unsigned char FALLING_RATE = 3; // drop every 250ms 5*50ms
const unsigned char JOYSTICK_SENSITIVITY = 10; // ignore 150ms after reading joystick 15*10ms

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

// Display String Func without clearing the screen first
void LCD_DisplayStringNoClear( unsigned char column, const unsigned char* string) { 
   unsigned char c = column;
   while(*string) {
      LCD_Cursor(c++);
      LCD_WriteData(*string++);
   }
}

// global vars vvv =============================================================================================
tetrisBlock *tb; // current block
int score;
// global vars ^^^ =============================================================================================

// tetris game funcs vvv =============================================================================================
bool gameOver() {
	if (checkBit(31,1))
		return true;
	return false;
}

bool canClearLine(int column, int bit) {
	for ( int i = 0; i < 16; i++){
		if ( !checkBit(column + i*4, bit) ){
			return false;
		}
	}
	return true;
}

bool clearLine (int line) {
	int column = line/5;
	int bit = (4 - line%5);
 	if (!canClearLine(column, bit)){
 		return false;
 	}
 	for ( int i = 1; i < 15; i++){
		clearPix(column + i*4, bit);
	}
	return true;
}

void shiftRow(int row, int bit) {
	switch (bit) {
		case 0:
			if (row % 4 ==  0){
				board[row] = board[row] | getBit(row+1, 4) | 0b10000;
			}
			else {
				board[row] = board[row] | getBit(row+1, 4);
			}
			break;
		case 1:
			if (row % 4 == 3){
				board[row] = ( (board[row] & 0b11110) << 1 ) | getBit(row, 4) << 4 | 0b00001 ;
			}
			else if (row % 4 ==  0){
				board[row] = board[row] << 1 | getBit(row+1, 4) | getBit(row, 3) << 3 | getBit(row, 2) << 2 | 0b10000;
			}
			else {
				board[row] = board[row] << 1 | getBit(row+1, 4) | getBit(row, 3) << 3 | getBit(row, 2) << 2 | getBit(row, 4) << 4;
			}
			break;
		case 2:
			if (row % 4 == 3){
				board[row] = ( (board[row] & 0b11110) << 1 ) | getBit(row, 4) << 4 | getBit(row, 3) << 3 | 0b00001;
			}
			else if (row % 4 ==  0){ 
				board[row] = board[row] << 1 | getBit(row+1, 4) | getBit(row, 3) << 3 | 0b10000;
			}
			else {
				board[row] = board[row] << 1 | getBit(row+1, 4) | getBit(row, 4) << 4 | getBit(row, 3) << 3;
			}
			break;
		case 3:
			if (row % 4 == 3){
				board[row] = ( (board[row] & 0b11110) << 1 ) | getBit(row, 4) << 4 | 0b00001;
			}
			else if (row % 4 ==  0){ 
				board[row] = board[row] << 1 | getBit(row+1, 4) | 0b10000;
			}
			else {
				board[row] = board[row] << 1 | getBit(row+1, 4) | getBit(row, 4) << 4;
			}
			break;
		default: // 4 or -1
			if (row % 4 == 3){
				board[row] = ( (board[row] & 0b11110) << 1 ) | 0b00001;
			}
			else if (row % 4 ==  0){
				board[row] = board[row] << 1 | getBit(row+1, 4) | 0b10000;
			}
			else {
				board[row] = board[row] << 1 | getBit(row+1, 4);
			}	
			break;
	}
}

void shiftLine(int line) {
	int column = line/5;
	int bit = (4 - line%5);
	for (int i = column; i < 4; i++){ // column
		for ( int j = 1; j < 15; j++){ // row
			if ( i == column ){
				shiftRow( (j*4) + i, bit );
			}
			else{
				shiftRow( (j*4) + i, -1 );
			}
		}	
	}

}

void removeLines() {
	for ( int i = 1; i < 19; i++){
		if (clearLine(i)){
			score++;
			shiftLine(i);
			i--;
		}
	}
}

void displayScore() {
	LCD_DisplayStringNoClear(5, (const unsigned char*) "SCORE:");
	LCD_WriteData('0' + score/10);
	LCD_WriteData('0' + score%10);
}

int getHighScore() {
	uint8_t tens = eeprom_read_byte((uint8_t*)46);
	uint8_t ones = eeprom_read_byte((uint8_t*)56);
	if (tens > 58 || ones > 58){ // if not a digit, default is larger
		return 0;
	}
	return ((tens-'0')*10 + (ones-'0'));
}

void setHighScore() {
	unsigned char t = score/10;
	unsigned char o = score%10;
	eeprom_write_byte ((uint8_t*)46 , t + '0');
	eeprom_write_byte ((uint8_t*)56 , o + '0');
}
// tetris game funcs ^^^ =============================================================================================

// tick funcs vvv =============================================================================================
unsigned char key;

int keyPadTick( int state ) {
	switch (state) {
		case 0: 
			return 1; break;
		case 1: 
			key = GetKeypadKey();
			if ( key != '\0'){
				if (key == '1'){
				 	for ( int i = 1; i < 15; i++){
						setPix(i*4, 3);
					}			
				}
				else if (key == '2'){
				 	for ( int i = 1; i < 15; i++){
						setPix(i*4, 2);
					}			
				}
				else if (key == '3'){
				 	for ( int i = 1; i < 15; i++){
						setPix(i*4, 1);
					}			
				}
				else if (key == '4'){
				 	for ( int i = 1; i < 15; i++){
						setPix(i*4, 0);
					}			
				}
				else if (key == '5'){
				 	for ( int i = 1; i < 15; i++){
						setPix(i*4 + 1, 4);
					}			
				}
				else if (key == 'A'){
					LCD_Cursor(32);
				 	LCD_WriteData( eeprom_read_byte((uint8_t*)56));			
				}
				else if (key == 'B'){
					LCD_Cursor(31);
				 	LCD_WriteData( eeprom_read_byte((uint8_t*)46));			
				}
				else if (key == 'C'){
					setHighScore();		
				}
				else if (key == '*'){
					score++;	
					displayScore();
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

enum TetrisStates { init, wait_start, show_hs, init_game, play, update_board, new_piece, win, game_over, check_hs, new_hs, new_game_wait, restart } tstate;

unsigned char tetrisCnt;
int tetrisTick( int state ) {
	switch (state) {
		case init: 
			LCD_DisplayString(1, (const unsigned char*) "PRESS");
			LCD_DisplayStringNoClear(17, (const unsigned char*) "START");
			return wait_start;
			break;
		case wait_start:
			if ( !(PINA & 0x08) ){
				LCD_ClearScreen();
				tetrisCnt = 0;
				return show_hs;
			}
			return wait_start;
		break;
		case show_hs: 
			LCD_DisplayString(1,(const unsigned char*) "HIGH SCORE:");
			LCD_WriteData(getHighScore()/10 + '0');
			LCD_WriteData(getHighScore()%10 + '0');
			LCD_DisplayStringNoClear(17, (const unsigned char*) "SCORE 20 TO WIN!!");
			return init_game;
		break;
		case init_game:
			if ( tetrisCnt > 40){
				LCD_ClearScreen();
				resetBoard();
				score = 0;
				displayBoard();
				displayScore();
				tetrisCnt = 0;
				tb = createTetrisBlock(rand() % 6);
				return play; 
			}
			tetrisCnt++;
			return init_game;
		break;
		case play: 	
			if ( !(PINA & 0x08) ){
				LCD_ClearScreen();
				tetrisCnt = 0;
				return init;
			}
			if (tetrisCnt < FALLING_RATE){
				tetrisCnt++;
			}
			else {
				tetrisCnt = 0;
				if (moveTetrisBlockDown(tb)){
					tetrisCnt = 0;
				}
				else{
					displayBoard();
					return update_board;
				}
			}
			displayBoard();
			return play;
		break;
		case update_board: 	
			removeLines();
			if (score >= 20){
				return win;
			}
			displayBoard();
			displayScore();
			return new_piece;
		break;
		case new_piece: 
			if (gameOver()){
				tetrisCnt = 0;
				return game_over;
			}	
			tb = createTetrisBlock(rand() % 6);
			displayBoard();
			return play;
		break;
		case win:
		LCD_DisplayString(1, (const unsigned char*) "YOU WIN!!!!!!!!:");
		LCD_DisplayStringNoClear(17, (const unsigned char*) "SCORE: 20");
		return check_hs;
		break;
		case game_over:
			LCD_ClearScreen();
			displayBoard();
			LCD_DisplayStringNoClear(5, (const unsigned char*) "GAME");
			LCD_DisplayStringNoClear(21, (const unsigned char*) "OVER");
			tetrisCnt = 0;
			return check_hs;
		break;
		case check_hs:
			if (tetrisCnt > 40){
				if ( score > getHighScore() ){
					setHighScore();
					return new_hs;
				}
				return restart;
			}
			tetrisCnt++;
			return check_hs;
		break;
		case restart:
			LCD_ClearScreen();
			LCD_DisplayString(1, (const unsigned char*) "PRESS START TO");
			LCD_DisplayStringNoClear(17, (const unsigned char*) "RESTART-SCORE:");
			LCD_WriteData('0' + score/10);
			LCD_WriteData('0' + score%10);
			resetBoard();
			return wait_start;
		break;
		case new_hs:
			LCD_DisplayString(1, (const unsigned char*) "NEW HIGH SCORE:");
			LCD_Cursor(17);
			LCD_WriteData('0' + score/10);
			LCD_WriteData('0' + score%10);
			tetrisCnt = 0;
			return new_game_wait;
		break;
		case new_game_wait:
			if (tetrisCnt > 40){
				return restart;
			}
			tetrisCnt++;
			return new_game_wait;
		break;
		default:
			return init;
	}
	return init;
}

const unsigned short lrMax = 1023;
const unsigned short udMax = 1023;

unsigned char joystickCnt;
unsigned char temp;
int joystickTick( int state ) {
	// read joystick into temp
	temp = 0;
	LCD_Cursor(12);
	if (ADC_read(0) > lrMax*3/4){
		temp = 1;
	}
	else if (ADC_read(0) < lrMax/4){
		temp = 2;
	}
	else if (ADC_read(1) > udMax*3/4){
		temp = 3;
	}
	else if (ADC_read(1) < udMax/4) {
		temp = 4;
	}

	switch (state) {
		case 0: 
			joystickCnt = 0;
			return 1; break;
		case 1: 
			if ( !(PINA & 0x04) ){
				while(moveTetrisBlockDown(tb));
				return 1;
			}	
			switch (temp) {
				case 1: moveTetrisBlockRight(tb); break;
				case 2: moveTetrisBlockLeft(tb); break;
				case 3: rotateTetrisBlock(tb); break;
				case 4: moveTetrisBlockDown(tb); break;
				default: break;
			}
			if (temp != 0){
				joystickCnt = 0;
				return 2;
			}
			return 1;
			break;
		case 2: // delay 10ms * JOYSTICK_SENSITIVITY
			if (temp == 0 || joystickCnt >= JOYSTICK_SENSITIVITY){
				joystickCnt = 0;
				return 1;
			}
			joystickCnt++;
			return 2;
			break;
		default:
			return 0;
	}
	return 0;
}
// tick funcs ^^^ =============================================================================================

int main(void) {
	// init ports
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xF0; PORTB = 0x0F; // Keypad Line
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines

	score = 0;
	srand(1);
	resetBoard(); // initializes board

	// tasks
 	static task task1, task2, task3;
    task *tasks[] = { &task1, &task2, &task3 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    task1.state = 0;
    task1.period = 50;
    task1.elapsedTime = task1.period;
    task1.TickFct  = &keyPadTick;

    task2.state = 0;
    task2.period = 10;
    task2.elapsedTime = task2.period;
    task2.TickFct  = &joystickTick;

    task3.state = init;
    task3.period = 50;
    task3.elapsedTime = task3.period;
    task3.TickFct  = &tetrisTick;

    unsigned short i;
    unsigned long GCD = tasks[0]->period;
    for ( i = 1 ; i < numTasks ; i++ ) {
    	GCD = findGCD(GCD, tasks[i]->period);
    }

	// init LCD and Timer
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

// avrdude -c atmelice_isp -p atmega1284 -B 5  -U flash:w:build/main.hex
// avrdude -p atmega2560 -P usb -c stk600 -B 5 -U flash:w:main.hex 
// avrdude -c atmelice_isp -p atmega1284 -c stk600 -B 5  -U flash:w:build/main.hex
