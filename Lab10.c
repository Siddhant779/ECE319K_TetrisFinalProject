// Lab10.c
// Runs on TM4C123
// Siddhant Pandit and Wesley Liu
// This is a starter project for the EE319K Lab 10

// Last Modified: 5/5/2021 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "SSD1306.h"
#include "Print.h"
#include "Random.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "TExaS.h"
#include "Switch.h"
#include "DAC.h"
//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PB54                  (*((volatile uint32_t *)0x400050C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PB5       (*((volatile uint32_t *)0x40005080)) 
#define PB4       (*((volatile uint32_t *)0x40005040)) 
#define PE3210    (*((volatile uint32_t *)0x4002403C)) 
#define PE1    (*((volatile uint32_t *)0x40024008))
#define PE0    (*((volatile uint32_t *)0x40024004))
#define busclock 80000000
#define sample 7000
#define con 4 // used for converting to pixels and the coordinates 
// TExaSdisplay logic analyzer shows 7 bits 0,PB5,PB4,PF3,PF2,PF1,0 
// edit this to output which pins you use for profiling
// you can output up to 7 pins

//sprite 
int imaindex; // used for the index of the image array
int sha; //for the index of the object 
int drawnow; // needed for drawing the actual image 
int gamenotdone; // signifies when the game is done - if game is done then its a one and displays the ending screen and score
int pos[28][12];
uint32_t score;


typedef enum {English, Spanish} Language_t;
Language_t myLanguage;
typedef enum {WELCOME, PRESS, SCORE, GAMEOVER} phrase_t;
const char Welcome_English[] ="Welcome! Press PE0";
const char Welcome_Spanish[] ="\xAD""Bienvenido! Pulsa";
const char Press_English[] = "for English.";
const char Press_Spanish[] = "PE1 para Espa\xA4ol.";
const char Score_English[]="Score: ";
const char Score_Spanish[]="Cuenta: ";
const char GameOver_English[]="Game over";
const char GameOver_Spanish[]="Fin del juego";
const char *Phrases[4][2]={
  {Welcome_English,Welcome_Spanish},
	{Press_English,Press_Spanish},
  {Score_English,Score_Spanish},
  {GameOver_English,GameOver_Spanish}
};

typedef enum {move,stop} status_t;
struct sprite{ 
	int32_t x;
	int32_t y;
	int32_t vx,vy;
	const uint8_t *image[4];
	status_t cond;
  int arr[4][4][4];
};
typedef struct sprite sprite_t;
sprite_t shapes[7];
// the first shape is l, second is clockwise l, third is counterclockwise l, fourth shape is the square, fifth shape is counterclockwise l, sixth shape is t, seventh shape is shaped z 
void Shape_Init(void){
	for(int t=0;t<28;t++){
		for(int m=0;m<12;m++){
		pos[t][m]=0;
	}
}
/*	int arr2[4][4][4] = { {{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,1,1,1}},
														 {{1,0,0,0},{1,0,0,0},{1,0,0,0},{1,0,0,0}},
														 {{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,1,1,1}},
														{{1,0,0,0},{1,0,0,0},{1,0,0,0},{1,0,0,0}} // this is str 
													 };
*/
	
	int arr2new [4][4][4]= { {{1,1,1,1},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
													 {{1,0,0,0},{1,0,0,0},{1,0,0,0},{1,0,0,0}},
													 {{1,1,1,1},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
													 {{1,0,0,0},{1,0,0,0},{1,0,0,0},{1,0,0,0}} //this is strnew
												 };
	
/*	int arr3[4][4][4] ={ {{0,0,0,0},{0,0,0,0},{1,1,0,0},{1,1,0,0}},
											 {{0,0,0,0},{0,0,0,0},{1,1,0,0},{1,1,0,0}},
											 {{0,0,0,0},{0,0,0,0},{1,1,0,0},{1,1,0,0}},
											 {{0,0,0,0},{0,0,0,0},{1,1,0,0},{1,1,0,0}} // this is sqr
										 };
												*/
	
	int arr3new[4][4][4]={ {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
												 {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
												 {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
												 {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}} //this is strnew
											 };
	
/*	int arr4 [4][4][4] ={ {{0,0,0,0},{0,0,0,0},{0,1,0,0},{1,1,1,0}},
												{{0,0,0,0},{1,0,0,0},{1,1,0,0},{1,0,0,0}},
												{{0,0,0,0},{0,0,0,0},{1,1,1,0},{0,1,0,0}},
												{{0,0,0,0},{0,1,0,0},{1,1,0,0},{0,1,0,0}} // this is tee
											};
											 */
	
	int arr4new [4][4][4]={ {{1,1,1,0},{0,1,0,0},{0,0,0,0},{0,0,0,0}},
													{{1,0,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}},
													{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
													{{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}} //this is teenew
												};
														
/*	int arr5 [4][4][4] = { {{0,0,0,0},{0,0,0,0},{1,0,0,0},{1,1,1,0}},
												 {{0,0,0,0},{1,1,0,0},{1,0,0,0},{1,0,0,0}},
												 {{0,0,0,0},{0,0,0,0},{1,1,1,0},{0,0,1,0}},
												 {{0,0,0,0},{0,1,0,0},{0,1,0,0},{1,1,0,0}} // this is lel
											 };
												*/
	int arr5new [4][4][4] = { {{1,1,1,0},{1,0,0,0},{0,0,0,0},{0,0,0,0}},
														{{1,0,0,0},{1,0,0,0},{1,1,0,0},{0,0,0,0}},
														{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
														{{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}} //this is lelnew
													};
	
/*	int arr6 [4][4][4] = { {{0,0,0,0},{0,0,0,0},{0,0,1,0},{1,1,1,0}},
												 {{0,0,0,0},{1,0,0,0},{1,0,0,0},{1,1,0,0}},
												 {{0,0,0,0},{0,0,0,0},{1,1,1,0},{1,0,0,0}},
												 {{0,0,0,0},{1,1,0,0},{0,1,0,0},{0,1,0,0}} // this is for rel
											 };
													*/
	int arr6new [4][4][4] = { {{1,1,1,0},{0,0,1,0},{0,0,0,0},{0,0,0,0}},
														{{1,1,0,0},{1,0,0,0},{1,0,0,0},{0,0,0,0}},
														{{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
														{{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}} // this is for relnew
													};
					
/*	int arr7 [4][4][4] = { {{0,0,0,0},{0,0,0,0},{1,1,0,0},{0,1,1,0}},
												 {{0,0,0,0},{0,1,0,0},{1,1,0,0},{1,0,0,0}},
												 {{0,0,0,0},{0,0,0,0},{1,1,0,0},{0,1,1,0}},
												 {{0,0,0,0},{0,1,0,0},{1,1,0,0},{1,0,0,0}} // this is for zee 
											 };
													*/
	int arr7new [4][4][4] ={ {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
													 {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},
													 {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
													 {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}} // this is for zeenew
												 };
	
/*	int arr8 [4][4][4] = { {{0,0,0,0},{0,0,0,0},{0,1,1,0},{1,1,0,0}},
												 {{0,0,0,0},{1,0,0,0},{1,1,0,0},{0,1,0,0}},
												 {{0,0,0,0},{0,0,0,0},{0,1,1,0},{1,1,0,0}},
												 {{0,0,0,0},{1,0,0,0},{1,1,0,0},{0,1,0,0}} // this is ess
											 };
												 */
	int arr8new [4][4][4] = { {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
														{{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}},
														{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
														{{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}}
													};
	for(int i=0;i<7;i++){
		shapes[i].x=16;
		shapes[i].y=32;
		shapes[i].vx=1;
		shapes[i].vy=0;
		shapes[i].cond=stop;
	}
	for(int a=0;a<4;a++){
			for(int b=0;b<4;b++){
				for(int c=0;c<4;c++){
					shapes[0].arr[a][b][c]=arr2new[a][b][c];
				}
		 }
	}
	for(int a=0;a<4;a++){
			for(int b=0;b<4;b++){
				for(int c=0;c<4;c++){
					shapes[1].arr[a][b][c]=arr3new[a][b][c];
				}
		 }
	}
	for(int a=0;a<4;a++){
			for(int b=0;b<4;b++){
				for(int c=0;c<4;c++){
					shapes[2].arr[a][b][c]=arr4new[a][b][c];
				}
		 }
	}
	for(int a=0;a<4;a++){
			for(int b=0;b<4;b++){
				for(int c=0;c<4;c++){
					shapes[3].arr[a][b][c]=arr5new[a][b][c];
				}
		 }
	}
	for(int a=0;a<4;a++){
			for(int b=0;b<4;b++){
				for(int c=0;c<4;c++){
					shapes[4].arr[a][b][c]=arr6new[a][b][c];
				}
		 }
	}
	for(int a=0;a<4;a++){
			for(int b=0;b<4;b++){
				for(int c=0;c<4;c++){
					shapes[5].arr[a][b][c]=arr7new[a][b][c];
				}
		 }
	}
	for(int a=0;a<4;a++){
			for(int b=0;b<4;b++){
				for(int c=0;c<4;c++){
					shapes[6].arr[a][b][c]=arr8new[a][b][c];
				}
		 }
	}
	
	shapes[0].image[0]= NewTetrisSTR1; //first image is a line 
	shapes[0].image[1]=NewTetrisSTR0;
	shapes[0].image[2]=NewTetrisSTR1;
	shapes[0].image[3]=NewTetrisSTR0; 
	
	shapes[1].image[0]=NewTetrisSQR0; // third image is a square
	shapes[1].image[1]=NewTetrisSQR0;
	shapes[1].image[2]=NewTetrisSQR0;
	shapes[1].image[3]=NewTetrisSQR0;
	
	shapes[2].image[0]=NewTetrisTEE3; //second image is a T
	shapes[2].image[1]=NewTetrisTEE0;
	shapes[2].image[2]=NewTetrisTEE1;
	shapes[2].image[3]=NewTetrisTEE2;
	
	shapes[3].image[0]=NewTetrisLEL3; //fourth image is the left one up 2 right
	shapes[3].image[1]=NewTetrisLEL0;
	shapes[3].image[2]=NewTetrisLEL1;
	shapes[3].image[3]=NewTetrisLEL2;
	
	shapes[4].image[0]=NewTetrisREL3; //fifth image is the right one up 2 left 
	shapes[4].image[1]=NewTetrisREL0;
	shapes[4].image[2]=NewTetrisREL1;
	shapes[4].image[3]=NewTetrisREL2;
	
	shapes[5].image[0]=NewTetrisZEE1; //sixth image is a z going to the left 
	shapes[5].image[1]=NewTetrisZEE0;
	shapes[5].image[2]=NewTetrisZEE1;
	shapes[5].image[3]=NewTetrisZEE0;
	
	shapes[6].image[0]=NewTetrisESS1; // seventh image is a z going to the right
	shapes[6].image[1]=NewTetrisESS0;
	shapes[6].image[2]=NewTetrisESS1;
	shapes[6].image[3]=NewTetrisESS0;
	
}
void ButtonInit(void){
	SYSCTL_RCGCGPIO_R|=0x10;
	while((SYSCTL_RCGCGPIO_R&0x10)!=0x10){};
	GPIO_PORTE_DIR_R&=~0x0F;
	GPIO_PORTE_DEN_R|=0x0F;
}
void Checkrow(void){
	// this function is done and right 
	for(int i=0;i<28;i++){ 
		int counter=0;
		for(int r=0;r<12;r++){	
			if(pos[i][r]==1){
				counter++;
		}
	 }
		if(counter==12){
			playsound(line);
			score++;
			int a=i;
			for(int s=i-1;s>0;s--){
				for(int j=0;j<11;j++){
					pos[a][j]=pos[s][j];
				
			}
				a--;
		}
			for(int t=0;t<11;t++){
			pos[0][t]=0;
		}
	}
 }
}
void Hit(void){
	//sees whether there was a hit or not 
	//also checks if the game is over
	for (int s=0;s<4;s++){
		for (int t=0;t<4;t++){
			if(shapes[sha].cond==move){
			if(shapes[sha].arr[imaindex][s][t]==1){
				//int colcon=((12-((shapes[sha].y-16)/4))+t);
				int colcon=(11-(((shapes[sha].y)-16)/con))+t;
				//int rowlcon=((shapes[sha].x)-((3-s)*4))/4;
				int rowlcon=((shapes[sha].x)/4 -s);
				//int rowlcon = (((shapes[sha].x)-((shapes[sha].x)-(s*4)))/4);
				if(rowlcon>=27){
					//then add it to hte pos array as that is the bottom 
					for(int q=0;q<4;q++){
						for(int t=0;t<4;t++){
							if(shapes[sha].arr[imaindex][q][t]==1){
								int col2con=((11-((shapes[sha].y-16)/con))+t);
							  //int rowl2con=((shapes[sha].x)-((3-q)*4))/4;
								//int rowl2con = (((shapes[sha].x)-((shapes[sha].x)-(q*4)))/4);
								int rowl2con=((shapes[sha].x)/4 -q);
								pos[rowl2con][col2con]=shapes[sha].arr[imaindex][q][t];
							}
						}
					}
					shapes[sha].cond=stop;
					playsound(bl);
			}
				if(pos[rowlcon+1][colcon]==1){
					//stop as it is about to hit another object 
					for(int w=0;w<4;w++){
						for(int e=0;e<4;e++){
							if(shapes[sha].arr[imaindex][w][e]==1){
								int col3con=((11-((shapes[sha].y-16)/con))+e);
								//int rowl3con=((shapes[sha].x)-((3-w)*4))/4;
								//int rowl3con = (((shapes[sha].x)-((shapes[sha].x)-(w*4)))/4);
								int rowl3con=((shapes[sha].x)/4 -w);
								pos[rowl3con][col3con]=shapes[sha].arr[imaindex][w][e];
							}
						}
					}
					playsound(bl);
					shapes[sha].cond=stop;
				}
		}
	}
	}
}
	for(int x=0;x<11;x++){
		if(pos[4][x]==1)
			gamenotdone=0;
	}
	
	Checkrow();
}
void Move(void){
	//this function should move the sprites - it is implemented by systick handler
	// it should set up the need to draw varaible as one
	// this function is right 
	shapes[sha].x+=shapes[sha].vx;
	int adcdata;
	int adcin=0;
	int temphigh=0;
	for(int j=3;j>=0;j--){
		for(int r=3;r>=0;r--){
			if(shapes[sha].arr[imaindex][j][r]==1){
				if(r>temphigh){
					temphigh=r;
				}
			}
				
		}
	}
	adcin=ADC_In();
	adcdata=adcin/342; // this gives the column 0-11
	if((11-adcdata)<temphigh){
		adcdata-=temphigh;
	}
	shapes[sha].y=63-(adcdata*con);
	drawnow=1;
	Hit();
}


void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PB54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
// edit this to initialize which pins you use for profiling
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x22;      // activate port B,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  GPIO_PORTF_DEN_R |=  0x0E;   // enable digital I/O on PF3,2,1
  GPIO_PORTB_DIR_R |=  0x30;   // output on PB4 PB5
  GPIO_PORTB_DEN_R |=  0x30;   // enable on PB4 PB5  
}
//********************************************************************************
 
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

void SysTick_Init(uint32_t period){
   NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = period-1;
	NVIC_ST_CURRENT_R = 0;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // changing the priority to 3 
	NVIC_ST_CTRL_R = 0x07;
}

int main(void){
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
	Shape_Init();
	ButtonInit();
	gamenotdone=1;
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
	ADC_Init(SAC_32);
	DAC_Init();
  SSD1306_ClearBuffer();
					
			SSD1306_OutBuffer();
	while(1){
		SSD1306_DrawBMP(0,63,TetrisMarquee,0,SSD1306_WHITE);
		SSD1306_OutBuffer();
	int done=1;
	int language=0; //default is english - if language==0 then its english if its equal to one then its spanish 
		gamenotdone=1;
		for(int i=0;i<8000000;i++){
			
		}
	while(done){
		SSD1306_ClearBuffer();
	SSD1306_DrawString(0,1,(char *)Phrases[WELCOME][English],SSD1306_WHITE);
	SSD1306_DrawString(0,10,(char *)Phrases[PRESS][English],SSD1306_WHITE);
	SSD1306_DrawString(0,20,(char *)"",SSD1306_WHITE);
	SSD1306_DrawString(0,30,(char *)Phrases[WELCOME][Spanish],SSD1306_WHITE);
	SSD1306_DrawString(0,40,(char *)Phrases[PRESS][Spanish],SSD1306_WHITE);
	SSD1306_OutBuffer();		// Display both welcome messages until a button input is made
		if(PE1==0x02 || PE0==0x01){
			done=0;
			if(PE1==0x02){
				language=1;
			}
		}
		SSD1306_ClearBuffer();
		//this clears the main screen 
		
	}
	done=1;
	for(int i=0;i<8000000;i++){}
	while(done){
	static uint32_t last0;
	static uint32_t last1;
	uint32_t now0= PE1>>1;
	uint32_t now1 = PE0;
	if(((now0==1) && (last0==0)) || ((now1==1) && (last1==0))){
			done=0;	
	}
	last0=now0;
	last1=now1;
		SSD1306_ClearBuffer();
	if(language == 0){	// language is English
		SSD1306_DrawString(0,1,(char *)"PE0 to speed up the ",SSD1306_WHITE);
		SSD1306_DrawString(0,10,(char *)"piece",SSD1306_WHITE);
		SSD1306_DrawString(0,20,(char *)"PE1 to rotate",SSD1306_WHITE);
		SSD1306_DrawString(0,30,(char *)"",SSD1306_WHITE);
		SSD1306_DrawString(0,40,(char *)"Press either button ",SSD1306_WHITE);
		SSD1306_DrawString(0,50,(char *)"to begin",SSD1306_WHITE);
	}
	else if(language==1){								// language is Spanish
		SSD1306_DrawString(0,1,(char *)"PE0 para acelerar la ",SSD1306_WHITE);
		SSD1306_DrawString(0,10,(char *)"pieza",SSD1306_WHITE);
		SSD1306_DrawString(0,20,(char *)"PE1 para girar",SSD1306_WHITE);
		SSD1306_DrawString(0,30,(char *)"",SSD1306_WHITE);
		SSD1306_DrawString(0,40,(char *)"Pulsa cualquiera de ",SSD1306_WHITE);
		SSD1306_DrawString(0,50,(char *)"los dos para comenzar",SSD1306_WHITE);
	}
	SSD1306_OutBuffer();
}
	
	for(int j=0;j<80000;j++){}
	for(int i=0;i<7;i++){
		shapes[sha].cond=stop;
	}
  /* 	for(int i=0;i<12;i++){
			pos[24][i]=0;
			SSD1306_DrawBMP(27*4,(63-(i*con)),TetrisBlock,0,SSD1306_WHITE);
		}	
		
		SSD1306_OutBuffer();
	*/
  	SSD1306_ClearBuffer();
	
/*	for(int j=0;j<12;j++){
		pos[27][j]=1;
	}
	for(int r=0;r<27;r++){
		pos[r][0]=1;
		pos[r][11]=1;
	}
	*/
	SysTick_Init(5000000);// replaced the 4 with 5 
	EnableInterrupts();
	Random_Init(NVIC_ST_CURRENT_R);
	while(gamenotdone){
		//playsound(theme);
		sha=((Random()+Random())*Random())%7;
		shapes[sha].cond=move;
		shapes[sha].x=16;
		shapes[sha].y=32;
		imaindex=0;
		shapes[sha].vx=1;
		SSD1306_ClearBuffer();
		SSD1306_OutBuffer();
			while(shapes[sha].cond==move)
			{
				if(drawnow){
					//this will continously draw out the game 
					SSD1306_ClearBuffer();
					for(int a=0;a<28;a++){
						for(int t=0;t<12;t++){
							if(pos[a][t]!=0){
								SSD1306_DrawBMP((a*con)+4,(63-(t*con)),TetrisBlock,0,SSD1306_WHITE);
							}
						}
					}
					for(int i=0;i<7;i++){
						if(shapes[sha].cond==move){
							SSD1306_DrawBMP(shapes[sha].x,shapes[sha].y,shapes[sha].image[imaindex],0,SSD1306_WHITE); 
							if(language==0){
								switch(score){
									case 0:
										SSD1306_DrawString(0,1,(char *)"Score:0",SSD1306_WHITE);
										break;
									case 1:
										SSD1306_DrawString(0,1,(char *)"Score:1",SSD1306_WHITE);
										break;
									case 2:
										SSD1306_DrawString(0,1,(char *)"Score:2 ",SSD1306_WHITE);
										break;
									case 3:
										SSD1306_DrawString(0,1,(char *)"Score:3 ",SSD1306_WHITE);
										break;
									case 4:
										SSD1306_DrawString(0,1,(char *)"Score:4 ",SSD1306_WHITE);
										break;
									case 5:
										SSD1306_DrawString(0,1,(char *)"Score:5 ",SSD1306_WHITE);
										break;
									case 6:
										SSD1306_DrawString(0,1,(char *)"Score:6 ",SSD1306_WHITE);
										break;
									case 7:
										SSD1306_DrawString(0,1,(char *)"Score:7 ",SSD1306_WHITE);
										break;
									case 8:
										SSD1306_DrawString(0,1,(char *)"Score:8 ",SSD1306_WHITE);
										break;
								}
							}
							else 
								switch(score){
									case 0:
										SSD1306_DrawString(0,1,(char *)"Puntaje:0 ",SSD1306_WHITE);
										break;
									case 1:
										SSD1306_DrawString(0,1,(char *)"Puntaje:1 ",SSD1306_WHITE);
										break;
									case 2:
										SSD1306_DrawString(0,1,(char *)"Puntaje:2 ",SSD1306_WHITE);
										break;
									case 3:
										SSD1306_DrawString(0,1,(char *)"Puntaje:3 ",SSD1306_WHITE);
										break;
									case 4:
										SSD1306_DrawString(0,1,(char *)"Puntaje:4 ",SSD1306_WHITE);
										break;
									case 5:
										SSD1306_DrawString(0,1,(char *)"Puntaje:5 ",SSD1306_WHITE);
										break;
									case 6:
										SSD1306_DrawString(0,1,(char *)"Puntaje:6 ",SSD1306_WHITE);
										break;
									case 7:
										SSD1306_DrawString(0,1,(char *)"Puntaje:7 ",SSD1306_WHITE);
										break;
									case 8:
										SSD1306_DrawString(0,1,(char *)"Puntaje:8 ",SSD1306_WHITE);
										break;
								}
							SSD1306_OutBuffer();
						}
					}
					drawnow=0;
				}
			}	
	}
	
	Shape_Init();
	SSD1306_ClearBuffer();	// Final screen displaying Game Over and score
	if(language == 0){
		SSD1306_DrawString(0,1,(char *)Phrases[GAMEOVER][English],SSD1306_WHITE);
	}
	else{
		SSD1306_DrawString(0,1,(char *)Phrases[GAMEOVER][Spanish],SSD1306_WHITE);
	}
	if(language==0){
								switch(score){
									case 0:
										SSD1306_DrawString(0,10,(char *)"Score:0 ",SSD1306_WHITE);
										break;
									case 1:
										SSD1306_DrawString(0,10,(char *)"Score:1 ",SSD1306_WHITE);
										break;
									case 2:
										SSD1306_DrawString(0,10,(char *)"Score:2 ",SSD1306_WHITE);
										break;
									case 3:
										SSD1306_DrawString(0,10,(char *)"Score:3 ",SSD1306_WHITE);
										break;
									case 4:
										SSD1306_DrawString(0,10,(char *)"Score:4 ",SSD1306_WHITE);
										break;
									case 5:
										SSD1306_DrawString(0,10,(char *)"Score:5 ",SSD1306_WHITE);
										break;
									case 6:
										SSD1306_DrawString(0,10,(char *)"Score:6 ",SSD1306_WHITE);
										break;
									case 7:
										SSD1306_DrawString(0,10,(char *)"Score:7 ",SSD1306_WHITE);
										break;
									case 8:
										SSD1306_DrawString(0,10,(char *)"Score:8 ",SSD1306_WHITE);
										break;
								}
							}
							else 
								switch(score){
									case 0:
										SSD1306_DrawString(0,10,(char *)"Puntaje:0 ",SSD1306_WHITE);
										break;
									case 1:
										SSD1306_DrawString(0,10,(char *)"Puntaje:1 ",SSD1306_WHITE);
										break;
									case 2:
										SSD1306_DrawString(0,10,(char *)"Puntaje:2 ",SSD1306_WHITE);
										break;
									case 3:
										SSD1306_DrawString(0,10,(char *)"Puntaje:3 ",SSD1306_WHITE);
										break;
									case 4:
										SSD1306_DrawString(0,10,(char *)"Puntaje:4 ",SSD1306_WHITE);
										break;
									case 5:
										SSD1306_DrawString(0,10,(char *)"Puntaje:5 ",SSD1306_WHITE);
										break;
									case 6:
										SSD1306_DrawString(0,10,(char *)"Puntaje:6 ",SSD1306_WHITE);
										break;
									case 7:
										SSD1306_DrawString(0,10,(char *)"Puntaje:7 ",SSD1306_WHITE);
										break;
									case 8:
										SSD1306_DrawString(0,10,(char *)"Puntaje:8 ",SSD1306_WHITE);
										break;
								}
			SSD1306_OutBuffer();
	for(int i=0;i<80000000;i++){}
}

}

// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}

void SysTick_Handler(void){ 
	//if htere is an error then its because you need to first find out if hte shape is moving or if its dead 
	int counter=0;
	for(int i=0;i<7;i++){
		if(shapes[i].cond==move)
		{
			counter++;
		}
	}
	
	while(counter>0){
	
	static uint32_t last0;
	static uint32_t last1;
	uint32_t now0= PE1>>1;
	if((now0==1) && (last0==0)){
		//if you want a faster response need to test both
	}
	if((now0==0) && (last0==1)){
			imaindex=((imaindex+1)%4);
			
	}
	last0=now0;
	uint32_t now1= PE0;
	if((now1==1) && (last1==0)){
		//if you want a faster response need to test both
	}
	if((now1==0) && (last1==1)){
			if(shapes[sha].vx==1){
				shapes[sha].vx=4;
			}
			else
				shapes[sha].vx=1;
			
	}
	counter=0;
	last1=now1;
	Move();
	
 }
}

