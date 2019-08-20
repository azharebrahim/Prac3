/*
 * BinClock.c
 * Jarrod Olivier
 * Modified for EEE3095S/3096S by Keegan Crankshaw
 * August 2019
 * 
 * <STUDNUM_1> <STUDNUM_2>
 * Date
*/
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h> //For printf functions
#include <stdlib.h> // For system functions

#include "BinClock.h"
#include "CurrentTime.h"

//Global variables
int cnt;
int hours, mins, secs;
long lastInterruptTime = 0; //Used for button debounce
int RTC; //Holds the RTC instance
int acnt;
int HH,MM,SS;

void myInterrupt(void) {
   //wiringPiI2CWriteReg8(RTC,HOUR, HOUR+1);

        printf("Button is Pressed");


   //printf(level);
}



void initGPIO(void){
	/* 
	 * Sets GPIO using wiringPi pins. see pinout.xyz for specific wiringPi pins
	 * You can also use "gpio readall" in the command line to get the pins
	 * Note: wiringPi does not use GPIO or board pin numbers (unless specifically set to that mode)
	 */
	printf("Setting up\n");
	wiringPiSetup(); //This is the default mode. If you want to change pinouts, be aware
	
	RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC
	
	//Set up the LEDS
	for(int i=0; i < sizeof(LEDS)/sizeof(LEDS[0]); i++){
	    pinMode(LEDS[i], OUTPUT);
	}
	
	//Set Up the Seconds LED for PWM
	//Write your logic here
	pinMode(SECS, PWM_OUTPUT);
	printf("LEDS done\n");
	
	//Set up the Buttons
	//for(int j=0; j < sizeof(BTNS)/sizeof(BTNS[0]); j++){
	//	pinMode(BTNS[j], INPUT);
	//	pullUpDnControl(BTNS[j], PUD_DOWN);
	//}
	
	//Attach interrupts to Buttons
	//Write your logic here
	pinMode(5, INPUT);
	pullUpDnControl(5, PUD_UP);
	wiringPiISR(5, INT_EDGE_RISING, &hourInc);
	
	pinMode(29, INPUT);
        pullUpDnControl(29, PUD_UP);
        wiringPiISR(29, INT_EDGE_RISING, &minInc);
	

	printf("BTNS done\n");
	printf("Setup done\n");

}
    
/*
 * The main function
 * This function is called, and calls all relevant functions we've written
*/ 
int main(void){
	//RTC = wiringPiI2CSetup(RTCAddr);
	initGPIO();
//	wiringPiI2CWriteReg8(RTC, 0x00, 0b10000000);
	//wiringPiISR(5, INT_EDGE_BOTH, &myInterrupt);
	//Set random time (3:04PM)
	//You can comment this file out later
///	wiringPiI2CWriteReg8(RTC, HOUR, 0x13+TIMEZONE);
//	wiringPiI2CWriteReg8(RTC, MIN, 0x4);
	//toggleTime();
	wiringPiI2CWriteReg8(RTC, 0x00, 0x80);
	// Repeat this until we shut down
	for (;;){
	
//Fetch the time from the RTC
		//Write your logic here
		hours =  wiringPiI2CReadReg8(RTC, HOUR);
		MM = wiringPiI2CReadReg8(RTC, MIN);
		secs = wiringPiI2CReadReg8(RTC,SEC);
		//Function calls to toggle LEDs
		//Write your logic here
		int b = hexCompensation(hours);
		lightHours(b);
		int a = hexCompensation(MM);
		lightMins(a);
		secs = secs - 0x80;

		secPWM(secs);
	//	toggleTime();
		// Print out the time we have stored on our RTC
		printf("The current time is: %x:%x:%x\n", hours, MM, secs);
		
		//using a delay to make our program "less CPU hungry"
		delay(1000); //milliseconds
	}
//	toggleTime();
	return 0;
}

/*
 * Change the hour format to 12 hours
 */
int hFormat(int hours){
	/*formats to 12h*/
	if (hours >= 24){
		hours = 0;
	}
	else if (hours > 12){
		hours -= 12;
	}
	return (int)hours;
}

/*
 * Turns on corresponding LED's for hours
 */
void lightHours(int units){
	// Write your logic to light up the hour LEDs here
digitalWrite(0, units & 1);
digitalWrite(2, units & 2);
digitalWrite(3, units & 4);
digitalWrite(25, units & 8);	
}

/*
 * Turn on the Minute LEDs
 */
void lightMins(int units){
	//Write your logic to light up the minute LEDs here

//hexCompensation(units);



digitalWrite(7, units & 1);
digitalWrite(22, units & 2);
digitalWrite(21, units & 4);
digitalWrite(27, units & 8);
digitalWrite(4, units & 16);
digitalWrite(6, units & 32);

}

/*
 * PWM on the Seconds LED
 * The LED should have 60 brightness levels
 * The LED should be "off" at 0 seconds, and fully bright at 59 seconds
 */
void secPWM(int units){

// Write your logic here
pwmWrite(1, units);
}

/*
 * hexCompensation
 * This function may not be necessary if you use bit-shifting rather than decimal checking for writing out time values
 */
int hexCompensation(int units){
	/*Convert HEX or BCD value to DEC where 0x45 == 0d45 
	  This was created as the lighXXX functions which determine what GPIO pin to set HIGH/LOW
	  perform operations which work in base10 and not base16 (incorrect logic) 
	*/
	int unitsU = units%0x10;

	if (units >= 0x50){
		units = 50 + unitsU;
	}
	else if (units >= 0x40){
		units = 40 + unitsU;
	}
	else if (units >= 0x30){
		units = 30 + unitsU;
	}
	else if (units >= 0x20){
		units = 20 + unitsU;
	}
	else if (units >= 0x10){
		units = 10 + unitsU;
	}
	return units;
}


/*
 * decCompensation
 * This function "undoes" hexCompensation in order to write the correct base 16 value through I2C
 */
int decCompensation(int units){
	int unitsU = units%10;

	if (units >= 50){
		units = 0x50 + unitsU;
	}
	else if (units >= 40){
		units = 0x40 + unitsU;
	}
	else if (units >= 30){
		units = 0x30 + unitsU;
	}
	else if (units >= 20){
		units = 0x20 + unitsU;
	}
	else if (units >= 10){
		units = 0x10 + unitsU;
	}
	return units;
}


/*
 * hourInc
 * Fetch the hour value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 23 hours in a day
 * Software Debouncing should be used
 */
void hourInc(void){
	//Debounce
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		printf("Interrupt 1 triggered, %x\n", hours);
		//Fetch RTC Time
		hours = wiringPiI2CReadReg8(RTC,HOUR);
		int b = hexCompensation(hours);
		//Increase hours by 1, ensuring not to overflow
		if (b<12)
		{
		b = b+1;
		}
		else
		{
		b= 0;
		}
		int c = decCompensation(b);
		wiringPiI2CWriteReg8(RTC,HOUR, c);
		//HH = c;
		//Write hours back to the RTC
		
	}
	lastInterruptTime = interruptTime;
}

/* 
 * minInc
 * Fetch the minute value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 60 minutes in an hour
 * Software Debouncing should be used
 */
void minInc(void){
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		printf("Interrupt 2 triggered, %x\n", mins);
		//Fetch RTC Time
		//mins = wiringPiI2CReadReg8(RTC,MIN);                
		//printf( " mins, %x\n ", mins);

		int g = hexCompensation(MM);
		
		//Increase minutes by 1, ensuring not to overflow
		if (g<60)
                {
                g = g+1;
                }
                else
                {
                g = 0;
                }

		int e = decCompensation(g);
                //printf( " mins, %x\n ", e);
		wiringPiI2CWriteReg8(RTC,MIN, e);
                //MM = c;

		//Write minutes back to the RTC
		
	}
	lastInterruptTime = interruptTime;
}

//This interrupt will fetch current time from another script and write it to the clock registers
//This functions will toggle a flag that is checked in main
void toggleTime(void){
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		HH = getHours();
		MM = getMins();
		SS = getSecs();

		HH = hFormat(HH);
		HH = decCompensation(HH);
		wiringPiI2CWriteReg8(RTC, HOUR, HH);

		MM = decCompensation(MM);
		wiringPiI2CWriteReg8(RTC, MIN, MM);

		SS = decCompensation(SS);
		wiringPiI2CWriteReg8(RTC, SEC, 0b00000000+SS);

	}
	lastInterruptTime = interruptTime;
}
