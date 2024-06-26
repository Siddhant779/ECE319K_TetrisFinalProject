// ADC.c
// Runs on TM4C123
// Provide functions that initialize ADC0
// Last Modified: 5//2021
// Student names: Siddhant Pandit and Wesley Liu
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(uint32_t sac){ 
	int delay;
	SYSCTL_RCGCGPIO_R |=0x08;
	while((SYSCTL_RCGCGPIO_R&0x08)==0){};
	GPIO_PORTD_DIR_R &=~0x04;
	GPIO_PORTD_AFSEL_R|=0x04;
	GPIO_PORTD_DEN_R&=~0x04;
	GPIO_PORTD_AMSEL_R|=0x04;
	SYSCTL_RCGCADC_R |=0x01;
	delay = SYSCTL_RCGCADC_R; // instead of delay
	delay = SYSCTL_RCGCADC_R; // instead of delay
	delay = SYSCTL_RCGCADC_R; // instead of delay
	delay = SYSCTL_RCGCADC_R; // instead of delay
	ADC0_PC_R=0x01;
	ADC0_SSPRI_R=0x0123;
	ADC0_ACTSS_R &=~0x0008;
	ADC0_EMUX_R &=~0xF000;
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5;
	ADC0_SSCTL3_R=0x0006;
	ADC0_IM_R &=~0x008;
	ADC0_ACTSS_R |= 0x0008;
	ADC0_SAC_R=sac;

}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
  uint32_t data;
	ADC0_PSSI_R=0x0008;
	while((ADC0_RIS_R&0x08)==0){};
	data=ADC0_SSFIFO3_R &0xFFF;
	ADC0_ISC_R=0x0008;
	return data;
}


