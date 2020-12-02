#include <platform.h>
#include <gpio.h>
#include <timer.h>
#include "delay.h"
#include <stdlib.h>

//When, EXPERIMENT_MODE is 0, the reaction time is 
//calculated by the time the LED turns on until the 
//user presses the User Button
//When value is 1, the reaction time is calculated
//by the time it took the user to turn on the LED.
#define EXPERIMENT_MODE  0	

int counter=0;
float avg_reaction_time=0;

static void Timer_Callback()
{
	counter++; 
}

int main() {
	
	int trial_repeat = 1;	//Counts how many times we repeat the experiment
	int timestamp = 50;	//50 us
	int total_reaction_time = 0;
	
	gpio_set_mode(PC_13, Input);	//SET User Button to Input 
	gpio_set_mode(PA_5, Output);	//SET LED to Output 
	gpio_set(PA_5, EXPERIMENT_MODE);
	
	timer_init(timestamp);
	
	//The callback function just increments a counter
	//periodicaly based on the timestamp value.
	timer_set_callback(&Timer_Callback);
	
	
	while (trial_repeat <= 5 ) {
		counter = 0;
		
		//The LED toggles randomly between a delay of 2 and 5 seconds
		delay_ms((rand()%3000)+2000);
		gpio_set(PA_5, !EXPERIMENT_MODE);
		timer_enable();
			while(1)
			{
				if(gpio_get(PC_13) == 0)
				{
					gpio_set(PA_5, EXPERIMENT_MODE);
					timer_disable();
					total_reaction_time += counter * timestamp ; 
					trial_repeat++ ; 
					break ; 
				}
				
			} 		
	}
	//avg_reaction_time gives us the average time it 
	//took the user to press the User Button in microseconds
	avg_reaction_time = total_reaction_time/(trial_repeat -1) ;
	
	//The following code is just so the user can understand 
	//whilst using the boeard, should the experiment is over
	//by seeing the LED blinking every half second.
	int bored = 0;
	while(bored < 5)
	{
		gpio_set(PA_5, !EXPERIMENT_MODE);
		delay_ms(500);
		gpio_set(PA_5, EXPERIMENT_MODE);
		delay_ms(500);
		bored++;
	}
	gpio_set(PA_5, 0);
}
