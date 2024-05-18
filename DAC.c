// dac.c
// This software configures DAC output
// Lab 6 requires a minimum of 4 bits for the DAC, but you could have 5 or 6 bits
// Runs on TM4C123
// Program written by: Siddhant Pandit and Wesley Liu
// Date Created: 5/1/21 
// Last Modified: 5/5/21 
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#define PB3210                  (*((volatile uint32_t *)0x4000503C)) // bits 5-4
	#define PF1       (*((volatile uint32_t *)0x40025008))

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
SYSCTL_RCGCGPIO_R |=0x02;
	__asm{
		NOP
		NOP
	}
	GPIO_PORTB_DIR_R |=0x0F;
	GPIO_PORTB_DEN_R |=0X0F;
	
	// this is for the LED part - put it part of the DAC init
/*	SYSCTL_RCGCGPIO_R |=0x20;
	__asm{
		NOP
		NOP
	}
	GPIO_PORTF_DIR_R |=0x01;
	GPIO_PORTF_DEN_R |=0x01;
	GPIO_PORTF_DIR_R |= 0x0E;
	GPIO_PORTF_DEN_R |= 0x0E;
	*/
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint32_t data){
	//GPIO_PORTB_DATA_R= data;
	GPIO_PORTB_DATA_R= data;
}
