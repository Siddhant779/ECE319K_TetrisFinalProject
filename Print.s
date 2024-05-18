; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; SSD1306_OutChar   outputs a single 8-bit ASCII character
; SSD1306_OutString outputs a null-terminated string 

    IMPORT   SSD1306_OutChar
    IMPORT   SSD1306_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix
    PRESERVE8
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

N EQU 4
CNT EQU 0
FP RN 11 

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
	PUSH{FP} ; saving the value from register 11
	PUSH{R0} ; storing the unsigned number on the stack
	SUB SP, #4 ; the space for count 
	MOV FP, SP ; making the frame pointer into R11 
	PUSH {LR}
	MOV R1, #0 ;making the count value 0 and storing it back
	STR R1, [FP,#CNT]
moDiv LDR R1, [FP,#CNT] ; this is the start of the read loop
	ADD R1, #1 ; adding one to the count value 
	STR R1, [FP,#CNT]
	MOV R1, #10 ; this will be for dividing the number 
	LDR R2, [FP,#N] 
	MOV R3,R2 ;making another copy of the number 
	UDIV R2, R2, R1 ; storing the divided number back into n
	STR R2, [FP,#N]
	MUL R2, R2, R1
	SUB R3, R3, R2
	PUSH{R3} ; storing the difference on the stack- contains the lowest weighted digit 
	LDR R1, [FP,#N]
	ADDS R1, #0
	BNE moDiv
wri	POP{R0} ; popping value from stack and it gets stored into R0 - this is the write loop
	ADD R0,R0, #0x30 ; making the ascii value 
	BL SSD1306_OutChar ;outputs the number
	LDR R1, [FP,#CNT] ;loading the count value to subtract 
	SUB R1, #1
	STR R1, [FP,#CNT]
	LDR R1, [FP,#CNT] ; checking if the value of count is 0 yet 
	CMP R1, #0
	BHI wri ; if the count value is still not 0 then loop back
	POP{LR}
	ADD SP, #8
	POP {R11}
	

  ; put your lab 7 solution here
      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.03 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
	PUSH{FP} ; saving the value from register 11
	PUSH{R0} ; storing the unsigned number on the stack
	SUB SP, #4 ; the space for count 
	MOV FP, SP ; making the frame pointer into R11 
	PUSH {LR}
	MOV R1, #0 ;making the count value 0 and storing it back
	STR R1, [FP,#CNT]
	MOV R1, #10 ; the ending of the initialization code 
	CMP R0,#1000 ; checking to see if the number is less than 1000
	BLO inRan ; if it is less than 1000 that means the value is in range 
	MOV R0, #0x2A ; outputs for if the value is not below 1000
	BL SSD1306_OutChar
	MOV R0, #0x2E
	BL SSD1306_OutChar
	MOV R0, #0x2A
	BL SSD1306_OutChar
	MOV R0, #0x2A
	BL SSD1306_OutChar
	B done 
	;these steps are steps 2 and 3 from the out dec 
inRan LDR R1, [FP,#CNT] ; this is the start of the read loop
	ADD R1, #1 ; adding one to the count value 
	STR R1, [FP,#CNT]
	MOV R1, #10 ; this will be for dividing the number 
	LDR R2, [FP,#N] 
	MOV R3,R2 ;making another copy of the number 
	UDIV R2, R2, R1 ; storing the divided number back into n
	STR R2, [FP,#N]
	MUL R2, R2, R1
	SUB R3, R3, R2
	PUSH{R3} ; storing the difference on the stack- contains the lowest weighted digit 
	LDR R1, [FP,#CNT] ; loading the count to the register 
	CMP R1, #3
	BLO inRan
	;same as step 4 in outdec 
	POP{R0} ; popping value from stack and it gets stored into R0 - this is the write loop
	ADD R0,R0, #0x30 ; making the ascii value 
	BL SSD1306_OutChar ;outputs the number
	MOV R0, #0x2E
	BL SSD1306_OutChar
	POP{R0} ; popping value from stack and it gets stored into R0 - this is the write loop
	ADD R0,R0, #0x30 ; making the ascii value 
	BL SSD1306_OutChar ;outputs the number
	POP{R0} ; popping value from stack and it gets stored into R0 - this is the write loop
	ADD R0,R0, #0x30 ; making the ascii value 
	BL SSD1306_OutChar ;outputs the number
	
done	
	POP{LR}
	ADD SP, #8
	POP {R11}
	
     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file
