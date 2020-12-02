#include <stdio.h>
#include "gpio.h"
#include "delay.h"
#include <stdlib.h>
#include "lcd.h"
#include <string.h>


//LED STATUS FLAGS
#define LED_ON 1
#define LED_OFF 0
//AVAILABLE LEDS (3 COLORS)
#define GREEN_LED 10
#define RED_LED 11
#define ORANGE_LED 12
//MAX NUMBER METRICS ALLOWED
#define MAX_NUM_TEMP_METRICS 24
//TEMPERATURE STATUSES (TEMP_NORMAL IS OMITTED)
#define TEMP_MELTDOWN 101
#define TEMP_OVERHEAT 100
#define TEMP_FROSTING 99


int delay = 50,sensor_time, temp_status = 0;
float temperature, temp_add = 0, distance;
uint8_t temp_byte1, temp_byte2 ,presence=0;
uint16_t TEMP;
//PINOUT
Pin temp_sensor = PA_12;
Pin red_led = PC_11;
Pin green_led = PC_5;
Pin orange_led = PB_9;
Pin buzzer_pin = PC_8;
Pin trigger = PC_9;
Pin echo = PA_6;

//WHEN CALLED THE MODE IS SET FOR EVERY
//DATA PIN OF THE U.S. SENSOR
void ultrasonic_sensor_start()
{
		gpio_set_mode(trigger, Output);
		gpio_set_mode(echo, PullDown);
}

/*IN ORDER TO CALCULATE DISTANCE THE U.S.
	SENSOR ACTIVATES THE TRIGGER (PC_9) WHICH 
	PRODUCES SHORT BURSTS OF ULTRASONIC SOUND 
	THAT ARE CAPTURED BY THE ECHO (PA_6). 
*/

float ultrasonic_sensor_read()
{
	int local_time = 0;
	gpio_set(trigger,1);
	delay_us(2);
	
	gpio_set(trigger,1);
	delay_us(10);
	gpio_set(trigger,0);
	
	while(!(gpio_get(echo)));
	while(gpio_get(echo))
	{
		local_time++;
		delay_us(1);
	}
	return local_time * .34 / 2;
}


uint8_t temperature_sensor_start()
{
	uint8_t Response = 0;
	gpio_set_mode(temp_sensor, Output);
	gpio_set(temp_sensor, 0);
	delay_us(480);
	gpio_set_mode(temp_sensor, Input);
	delay_us(80);
	if(!(gpio_get(temp_sensor)))
	{
		Response = 1;
	}
	else
	{
		Response = -1;
	}
	delay_us(400);
	return Response;
}

void temperature_sensor_write(uint8_t data)
{
	gpio_set_mode(temp_sensor, Output);
	for(int i=0; i<8; i++)
	{
		if((data & (1<<i))!=0)
		{
			gpio_set_mode(temp_sensor, Output);
			gpio_set(temp_sensor, 0);
			delay_us(1);
			gpio_set_mode(temp_sensor, Input);
			delay_us(delay);
		}
		else
		{
			gpio_set_mode(temp_sensor,Output);
			gpio_set(temp_sensor, 0);
			delay_us(delay);
			gpio_set_mode(temp_sensor, Input);
		}
	}
}

uint8_t temperature_sensor_read()
{
	uint8_t value = 0;
	gpio_set_mode(temp_sensor, Input);
	for(int i=0; i<8; i++)
	{
		gpio_set_mode(temp_sensor, Output);
		gpio_set(temp_sensor,0);
		delay_us(1);
		gpio_set_mode(temp_sensor, Input);
		if((gpio_get(temp_sensor)))
		{
			value |= 1<<i;
		}
		delay_us(delay);
	}
	return value;
}

float temp_sensor_pulse()
{
	presence = temperature_sensor_start();
	delay_cycles(1);
	temperature_sensor_write(0xCC); //skip ROM
	temperature_sensor_write(0x44); //convert t
	delay_cycles(800);
	presence = temperature_sensor_start();
	delay_cycles(1);
	temperature_sensor_write(0xCC); //skip ROM
	temperature_sensor_write(0xBE); //read scratch-pad
	temp_byte1 = temperature_sensor_read();
	temp_byte2 = temperature_sensor_read();
	TEMP = (temp_byte2<<8)|temp_byte1;
	return (float)TEMP/16;
}


void led_op(int color,int mode)
{
	gpio_set_mode(red_led, Output);
	gpio_set_mode(green_led, Output);
	gpio_set_mode(orange_led, Output);
	switch (color)
	{
		case GREEN_LED:
		{
			switch(mode)
			{
				case LED_OFF:
				{
					gpio_set(green_led,LED_OFF);
					break;
				}
				case LED_ON:
				{
					gpio_set(green_led, LED_ON);
					break;
				}
			}
			break;
		}
		case RED_LED:
		{
			switch(mode)
			{
				case LED_OFF:
				{
					gpio_set(red_led,LED_OFF);
					break;
				}
				case LED_ON:
				{
					gpio_set(red_led, LED_ON);
					break;
				}
			}
			break;
		}
		case ORANGE_LED:
		{
			switch(mode)
			{
				case LED_OFF:
				{
					gpio_set(orange_led,LED_OFF);
					break;
				}
				case LED_ON:
				{
					gpio_set(orange_led, LED_ON);
					break;
				}
			}
			break;
		}
	}
}


void print_temp_metrics(int temp_counter)
{
		char temp[10],avg[10];
		float average = temp_add / temp_counter;
		lcd_set_cursor(0,0);
		sprintf(temp,"%.2f",temperature);
		lcd_print("Temp oC: ");
		lcd_set_cursor(11,0);
		lcd_print(temp);
		lcd_set_cursor(0,1);
		sprintf(avg,"%.2f",average);
		lcd_print("Temp AVG: ");
		lcd_set_cursor(11,1);
		lcd_print(avg);
}

void get_temperature()
{
		temperature = temp_sensor_pulse();
		float temporary = temperature + temp_add;
		temp_add = temporary;
		if(temperature < 26.5)
		{
			led_op(GREEN_LED,LED_ON);
			led_op(RED_LED,LED_OFF);
			led_op(ORANGE_LED, LED_OFF);
			temp_status = TEMP_FROSTING;
		}
		else if(temperature > 27.5 && temperature < 29)
		{
			led_op(GREEN_LED,LED_OFF);
			led_op(RED_LED,LED_OFF);
			led_op(ORANGE_LED, LED_ON);
			temp_status = TEMP_OVERHEAT;
		}
		else if(temperature >= 29)
		{
			led_op(GREEN_LED,LED_OFF);
			led_op(RED_LED,LED_ON);
			led_op(ORANGE_LED, LED_OFF);
			temp_status = TEMP_MELTDOWN;
		}
		else
		{
			led_op(GREEN_LED,LED_OFF);
			led_op(RED_LED,LED_OFF);
			led_op(ORANGE_LED, LED_OFF);
			temp_status = 0;
		}
}


int main() {
	lcd_init();
	int temp_counter = 0, repeats = 0 , buzzer_active = 0; 
	gpio_set_mode(buzzer_pin,Output);
	ultrasonic_sensor_start();
	while(1)
	{
		get_temperature();
		temp_counter++;
		if(temp_counter == MAX_NUM_TEMP_METRICS)
			{
				char temp[10];
				lcd_clear();
				lcd_set_cursor(0,0);
				lcd_print("Temperature AVG:");
				lcd_set_cursor(0,1);
				sprintf(temp, "%.2f", temp_add/MAX_NUM_TEMP_METRICS);
				lcd_print(temp);
				lcd_set_cursor(7,1);
				lcd_print("oC");
				delay_ms(10000);
				temp_add = 0;
				temp_counter = 0;
			}
		
		while (repeats < 20)
		{
				delay_ms(122);
				if(buzzer_active == 1)
				{
					gpio_set(buzzer_pin,1);
				}
				
				if(ultrasonic_sensor_read() < 15)
				{
					lcd_clear();
					print_temp_metrics(temp_counter);
				}
				else
				{
					switch(temp_status)
					{
						case TEMP_OVERHEAT:
						{
							lcd_clear();
							lcd_set_cursor(0,0);
							lcd_print("WARNING !");
							lcd_set_cursor(0,1);
							lcd_print("OVERHEAT");
							buzzer_active = 0;
							break;
						}
						case TEMP_FROSTING:
						{
							lcd_clear();
							lcd_set_cursor(0,0);
							lcd_print("WARNING !");
							lcd_set_cursor(0,1);
							lcd_print("FROSTING");
							buzzer_active = 0;
							break;
						}
						case TEMP_MELTDOWN:
						{
							lcd_clear();
							lcd_set_cursor(0,0);
							lcd_print("DANGER !");
							lcd_set_cursor(0,1);
							lcd_print("MELTDOWN");
							buzzer_active = 1;
							break;
						}
						default:
						{
							lcd_clear();
							lcd_set_cursor(0,0);
							lcd_print("THERMOSTAT");
							lcd_set_cursor(0,1);
							lcd_print("ACTIVE");
							buzzer_active = 0;
							break;
						}
					}
				}			
			delay_ms(123);
			repeats++;
			gpio_set(buzzer_pin,0);
		}
		repeats = 0;		
	}
}

