/********************************************************************
UART library for using the UART of LPC1768 (ARM Cortex M3), 
this has been tested for the Blue Board

Author   - Aman Kanwar
Verision - 1.0

--- APIs provided by this Library----------
 sendCharacter(unsigned char)    -- To Send a character on the  UART3
 sendString(unsigned char *)     -- To Send a character on the  UART3
 receiveCharacter(void)          -- To receive a character from UART3
 nextLine(unsigned char)         -- To insert a newline on the terminal
 delay(unsigned int)             -- input a delay
 receiveString(unsigned char*)   -- To receive a String from the UART3
 receivePassword(unsigned char*) -- To receive a Password from the UART3
-------------------------------------------

*********************************************************************/


#ifndef __UART_CORTEXM3_H__
#define __UART_CORTEXM3_H__

#include "lpc17xx.h"

//-------------- UART BIT MAPPINGS --------------------------------
#define PCUART3      25
#define PCLK_UART3   18
//-----------------------------------------------------------------

//----------  UARTn Divisor Latch LSB register (U0DLL, U1DLL, U2DLL, U3DLL) --------------
#define DLLSB        0
//----------------------------------------------------------------------------------------
//----------  UARTn Divisor Latch LSB register (U0DLM, U1DLM, U2DLM, U3DLM) --------------
#define DLMSB        0
//----------------------------------------------------------------------------------------


//------------- UARTn Line Control Register (for U0LCR, U1LCR, U2LCR, U3LCR) -------------
#define Word_Length_Select    0      // bit 1:0
#define Stop_Bit_Select       2      // bit 2 
#define Divisor_Latch_Access  7      // bit 7
//----------------------------------------------------------------------------------------


//-------------------Status Check Macros --------------------------
#define BIT_STATUS(REGISTER,BIT) ((REGISTER &(1<<BIT)))
//-----------------------------------------------------------------

//------------------- Macro to SET and Clear a BIT ----------------
#define HIGH 1
#define SBIT(REGISTER,BIT,VALUE) (REGISTER |(VALUE<<BIT))
#define CLR(REGISTER,BIT,VALUE) (REGISTER &(~(VALUE<<BIT)))
//-----------------------------------------------------------------

//------------------- Uart Pins for RXD3 and TXD3 -----------------
// bit number, in these bits, we will write the function value (00,01,10,11)                     
#define PINSEL0_RXD3   2
#define PINSEL0_TXD3   0
#define PINSEL1_RXD3  20
#define PINSEL1_TXD3  18
#define PINSEL9_RXD3  26
#define PINSEL9_TXD3  24
//-----------------------------------------------------------------

//-------- UART LINE STATUS REGISTER MACROS -----------------------------------
//					Bit Number
#define THRE    5        // Transmit Holding Register Empty
#define RDR     0        // Receiver Data Ready 


#define PREVIOUS_DATA_IS_NOT_SENT (!(BIT_STATUS(LPC_UART3->LSR,THRE)))
// when THRE-0 THR (transmitter hold register) contains valid data, THRE-1 THR is empty

#define DATA_IS_NOT_AVAILABLE     (!(BIT_STATUS(LPC_UART3->LSR,RDR)))   
// when RDR-0 receiver is empty, when RDR-1,receiver is not empty

//-----------------------------------------------------------------------------

void init_UART(void);

void sendCharacter(unsigned char);
void sendString(unsigned char *);
unsigned char receiveCharacter(void);
void nextLine(unsigned char);
void delay(unsigned int);
void receiveString(unsigned char*);
void receivePassword(unsigned char*);

void receivePassword(unsigned char* charArray)
{
	unsigned char index =0;
  unsigned char data;
	
	while(data!='\r')
	{
		data = receiveCharacter();
		sendCharacter('*');
		charArray[index] = data;
		++index;
	}
	charArray[index] = '\0';       // adding NULL at the ending of the current string to avoid garbage printing
}


void receiveString(unsigned char* charArray)
{
	unsigned char index =0;
  unsigned char data;
	
	while(data!='\r') 
	{
		data = receiveCharacter();
		sendCharacter(data);
		charArray[index] = data;
		++index;
	}
	charArray[index] = '\0';       // adding NULL at the ending of the current string to avoid garbage printing
}


void delay(unsigned int time)
{
for(int i=0;i<1000;i++)
			{
				for(int j=0;j<time;j++);
			}
}


// This funciton block, received a single character from the user
unsigned char receiveCharacter()
{
 unsigned char userInput;	
 while(DATA_IS_NOT_AVAILABLE);    // wait to receive a character
 userInput = LPC_UART3->RBR;      // Read Value from the receiver buffer register
 return(userInput);               // return the received value to the caller
}


//------------This function block will be send any character (Value provided as parameters by the caller)
void sendCharacter(unsigned char data)
{
  while(PREVIOUS_DATA_IS_NOT_SENT);   // wait for the THRE to become 1, i.r THR to becomes empty
	LPC_UART3->THR = data;              // sending the data to the THR register
}
//-----------------------------------------------------------------------------

void nextLine(unsigned char numberOfLines)
{
// below mentioned code lines will insert a new line on the terminal
  while(numberOfLines > 0)
	{
	sendCharacter('\r');   // carriage return
  sendCharacter('\n');	
  --numberOfLines; 		
  }
}

//------------This function block will be send any character (Value provided as parameters by the caller)
void sendString(unsigned char *dataString)
{
	unsigned char index = 0;
  while(dataString[index] != '\0'){sendCharacter(dataString[index++]);}
	// in the above line of code, we are sending each character present in the "dataString"
}
//-----------------------------------------------------------------------------


void init_UART()
{
	// Enabling the UART 3 in PCONP register and selecting the 1/4th of CCLK for PCLK
  LPC_SC->PCONP        |=      SBIT(LPC_SC->PCONP,PCUART3,HIGH); 	       // enabling the UART3
	LPC_SC->PCLKSEL1     |=      CLR(LPC_SC->PCLKSEL1,PCLK_UART3,HIGH);    // to clear the PINS 18,19	
  
	//(to setup Line Control Register, Divisor_Latch_Access should be high)
  LPC_UART3->LCR       |=      SBIT(LPC_UART3->LCR,Divisor_Latch_Access,HIGH);
	// Setting up the Line Control Register, 
	LPC_UART3->LCR       |=      SBIT(LPC_UART3->LCR,Word_Length_Select,3);    
	//LPC_UART3->LCR       |=      CLR(LPC_UART3->LCR,Stop_Bit_Select,HIGH);    

	/*now setting up the DLL, 
	(make sure that DLAB =1 to enables access to registers DLL and DLM for setting the baud rate. */
	// UARTn Divisor Latch LSB register (U0DLL)
	LPC_UART3->DLL        = 162;   // for baud rate of 9600,
  LPC_UART3->DLM        =   0;	
	/*
	DLL =     PCLK     (PCLK is 25 MHz, as calculated earlier, 100/4 = 25MHz)
					---------	 (Assuming DLM =0, U3FDR=0x01 i.e. MUL and DIV values are 1 and 0 rspectively)
	        16 x 9600
	*/
	
	//(once the LCR and Baud Rate is set, Divisor_Latch_Access should be low)
    LPC_UART3->LCR        =      CLR(LPC_UART3->LCR,Divisor_Latch_Access,HIGH);
 		
	//LPC_UART3->LCR        = 0x03;
		
	 /*Next step is to setup the FIFO settings in "UARTn FIFO Control Register", since the default
	   values in this registers are set to 0 and we do not need FIFO in UART,
		 Hence, we need not to modify this register
		 In case, if PINMODE is used then 
		 make sure that the UART receive pins should not have pull-down resistors enabled*/
   
	 // setting the PINSEL value for the given Pins, (P0.1 -- RXD3 and P0.0 -- TXD3) 
	 LPC_PINCON->PINSEL0  |=     SBIT(LPC_PINCON->PINSEL0,PINSEL0_RXD3,2) |
                               SBIT(LPC_PINCON->PINSEL0,PINSEL0_TXD3,2);  	 
  
	 // now we just need to send and receive the data
}

#endif
