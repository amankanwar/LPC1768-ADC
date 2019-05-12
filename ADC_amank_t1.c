/*############# This program is for programming the LPC1768 (ARM Cortex M3) ADC #####################

Author- Aman Kanwar

Project Title:- Using the ADC on the Blueboard and based on the current value of the 
Potentiometer (analog source), the ADC of LPC1768 should perform the following actions

1:- Do the conversion contineously
2:- Display the Value of the Potentiometer on LCD, Uart (in Volts)
####################################################################################################*/


// ===>>  In this code, I have tried to explain each and every step (like initializations, etc) breifly <<=== //

/**************   KEY NOTES *********************
1:	On the BlueBoad the given Potentiometer, which we are using as the Analog Voltage source is connected
		with the ADC ADC0.2 (channel 2), hence for a different board, some modifications might be required

2:	The value of CCLK is is 100MHz (as we be verified in the Source file of the ADC)

3:	The Baud Rate of the Uart is 9600 (for now), which can be modified later on!
*************************************************/



#include "lpc17xx.h"
#include "Uart_CortexM3.h"
#include "stdio.h"
#include "lcd.h"


	#define HIGH 			    1					// for bit shifting operation
	#define CLKDIV_VAL	        1					// value for CLKDIV (based on our calculation)
	#define AD_SEL_2		    2					// To select the the ADC Channel 2
	
//			PinName			PinNumber	    	            	SIGNIFICANCE						       	Default Value						
	#define PCADC 			    12					//	A/D converter (ADC) power/clock control bit.			0
	#define PDN					21					//	Set the bit to make the A/D converter operational 	    0
	#define PCLK_ADC		    24					//	Here there are two pins 24 and 25 for ADCPCLK 			0 (CCLK/4)
	#define AD0_2				18					//	Performs AD0.2 Function when 01	value in PINSEL1		0
	#define CLKDIV		 	     8					//	to produce the clock for the A/D converter				0
	#define BURST				16					//	Set this bit for ADC to do repeated conversions			0
	#define ADGINTEN		     8					//	individual ADC channels will generate interrupts.		1 
	#define DONE2				 2					//	DONE status flag from the result for A/D channel 2.     0
	#define SEL					 2					//	Selects which AD0.7:0 to be sampled and converted		0


//-------------This function will initialize the ADC, set the values and make the ADC operational --------------------------------------
void init_ADC()
{	
    //  STEP 1:-	In the PCONP register (Table 46), set the PCADC bit.
			LPC_SC->PCONP				|=	(HIGH	<< 	PCADC);
	
	//	STEP 2:-	In the A/D Control Register (AD0CR), make the ADC operational 
			LPC_ADC->ADCR				|=	(HIGH	<<	PDN);
			LPC_ADC->ADCR				|=	(HIGH	<<	SEL);			//SELECTING CHANNEL 2 of ADC0.2
	
    /*	STEP 3:-	            This is an important step, wherein we need to set the working frequency
								for our ADC peripheral. Here, we need to select the Clock frequency for
								ADC by Dividing the CCLK (100 MHz in our case) with either 4,1,2,8
								this depends on the value we provide to the PCLKSEL0 register (Pins 25:24) 
					
								Since the default value is 0, So the PCLK for ADC is CCLK/4 = 25 MHz
                            	Hence, we need not to modify these two bits (25:24)*/
	
	/*	STEP    4:-		        Setting the CLKDIV value, APB clock (PCLK_ADC0) will be divided by Value in CLKDIV+1
								and the resultand frequency will be used by the ADC (make sure resultant is <= 13 MHz) 
								Since, defauld Value of APB clock (PCLK_ADC0) is 25 MHz, hence we will provide CLKDIV
								a value = 1, hence the result frequency will be PCLK_ADC0/(CLKDIV+1), 25/(1+1) = 12.5 MHz */			
			LPC_ADC->ADCR				|=	(HIGH	<<	CLKDIV);
	

    // 	STEP5:-			        Important!!! here we will be selecting the ADC Channel, (2 in our case, )
			LPC_ADC->ADCR				|=	(HIGH	<<	SEL);

	
	//	STEP6:-		            Setting ADC0 pins function using PINSEL register.	For our BlueBoard, its ADC0.2 channel	
			LPC_PINCON->PINSEL1	        |=	(CLKDIV_VAL	    <<	AD0_2);
	
	/*	STEP7:-		            Setting the Conversion Mode For our ADC, here we may either select the Burst Mode (Contineous Conversion)
								, or we may select the normal conversion mode, wherein the conversion will  be done only once,
								In this code, we are using the burst mode for the contineous conversion of the data coming form the POT 
								Also, when this bit is set (and the Start Bits are set to 00), conversion starts
								
								As recommended in the DataSheet, we are also clearing the ADGINTEN bit in the AD0INTEN register
								this will make sure that Only the individual ADC channels enabled by ADINTEN7:0 will generate interrupts. */			
			LPC_ADC->ADINTEN		    &=	 ~(HIGH     <<	ADGINTEN);
			LPC_ADC->ADCR				|=    (HIGH	    <<	BURST);	

}
//---------------------------------------------------------------------------------------------------------------------------------------------


//----------------This function will read the data from data register, perform the proper shifting and return data -----------
unsigned short int capture_value()
{
	
	unsigned short int data;
	/*	in order to check for the conversion of the data (polling approach), we will monitor the flags in ADSTAT register
			In our case, since we are using the Channel 2 of the ADC0, which is connected to the POT */
		
	//	we will wait for the ADC to complete the conversion, hence we are monitoring the DONE2 flag in ADCSTAT register	
	while(((LPC_ADC->ADSTAT) & (HIGH << DONE2)) != (HIGH << DONE2)){}
	
	/*	once the conversion is done, we will capture the data from the ADC Data Register, (from RESULT bits 15:4) 
			
			IMPORTANT !!!, in order to use the data, we need to mask the RESULT bits (15:4) and shift the data to MSB, since
			the data will be apprearing on the bits 15 (LSB) :4 (MSB) */
			data	=	(((LPC_ADC->ADDR2) & (0x0FFF0)) >> 4 );
	//	in the above peice of code we have Masked the data and shifter the data to the right 4 times, now LSB is at bit 0
		
	return(data);
}
//----------------------------------------------------------------------------------------------------------------------------


//------ This function will perform the purpose of data conversion and returning data to the caller-------------
double converted_voltage(double step_value)
{
		unsigned short int conData;
		if((conData = capture_value())== 0){return(0);}     // return zero (0) if there is no analog voltage.
		return(((conData +1) * step_value));                // if non-zero then perform conversion and return.
}	
//--------------------------------------------------------------------------------------------------------------


//--------------------------------This function will Update a ADC data on the Serial Terminal -------------------
void adcValueMenu(char *sdata)
{
    //------------- UART APIs ----------------------------------------
	sendString((unsigned char *)"   __________===> Value of ADC: ");	
	sendString((unsigned char *)sdata);
	sendString((unsigned char *)" Volts");
	sendString((unsigned char *)" <===__________");
	sendCharacter('\r');
    //----------------------------------------------------------------
}
//---------------------------------------------------------------------------------------------------------------


//****************** MAIN FUNCTION *********************************************************************	
int main()
{
	
    //-------------Initializations-------------------------------
	init_ADC();                         // Initializing the ADC
	init_UART();                        // Initializing the UART
	init_lcd();                         // Initializing the LCD
	send_cmd(0x01);						// Clear the LCD
	send_cmd(0x80);						// force the LCD's cursor on the position 1st Row 1st column
    //-----------------------------------------------------------
 	
    char sdata[2];                      // Array of String to hold the converted data (later)
  
    //------------- Using the UART APIs ---This will show a starting messge on Serial Terminal-----------
	nextLine(1);
	sendString((unsigned char *)"======= ADC  CONVERTOR By Aman Kanwar==========");
	nextLine(5);
    sendString((unsigned char *)" ---------------- Polling Approach ----------------------------");
	nextLine(5);
	sendString((unsigned char *)" Kindly move the Potentiometer Probe to see changes");
	nextLine(5);
    //---------------------------------------------------------------------------------------------------	

    /*--------------------------------------------------------------------------------------------------
    	Since our adc is of 12 bit Values will range from 0 - 4095 (steps), voltage change per step is 
	
    	    0.0008056640625     volts/Per Step
		or 	0.8056640625 		milli volts/Per Step
		or	805.6640625 		micro volts/Per Step  
	
			Hence we need to perform the conversion and display the data on the LCD and Uart
            For this conversion, we have built a special funciton named converted_voltage()	
	---------------------------------------------------------------------------------------------------*/

    //__________________________________________________________________________________________________
	while(1)
	{
		sprintf(sdata,"%lf",converted_voltage(0.0008056640625));
		
		//--------Sending the Data On Uart -------------------------------
		adcValueMenu(sdata);
		//----------------------------------------------------------------
				
		//--------Sending the Data On LCD ---------------------------------
		send_cmd(0x01);													// Clear the LCD's previous data			
		user_string((unsigned char *)sdata);
		user_string((unsigned char *)" Volts");
		delay(3500);
		//-----------------------------------------------------------------
	}
    //__________________________________________________________________________________________________

return 0;
}
//*********************************************************************************************************	
