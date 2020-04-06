/*
 * Calculator.c
 *
 * Created: 4/3/2020 3:43:19 PM
 * Author : AL-alamia
 */ 

#include <avr/io.h>
#define F_CPU 8000000
#include <util/delay.h>
#include "avr/io.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "util/delay.h"
#include "lcd_4bit.h"

static int result = 0;
static int num1 = 0,num2 = 0;
/* Define Tasks Priorities */
#define  TASK1_PRIORITY (3)
#define  TASK2_PRIORITY (2)

void vApplicationIdleHook( void );

void Welcome(void * pvParameters);
void Task2(void * pvParameters);
void Calculator(void * pvParameters);
void KEYPAD_Scanner(void * pvParameters);

void LCD_DisplayPattern(void);
void KEYPAD_Scan(void);
void LCD_Display_KP(void);

xTaskHandle handle_task1;
xTaskHandle handle_task2;
xTaskHandle handle_Calculator;
xTaskHandle handle_KEYPAD_Scanner;
xTaskHandle handle_BUTTON_Tracker;
xTaskHandle handle_LED_Blinker;
void display_welcome();
void waiting_for_key();
void KEYPAD_Scan(void);
char pressed_number = 255;
typedef enum{GetNUM1,GetOperation,GetNUM2,Calculate}CALC_STATES_t;
typedef enum{ADD,SUB,MUL,DIV}OPERATIONS_t;
OPERATIONS_t operation = ADD;
CALC_STATES_t CALC1 = GetNUM1;
void BUTTON_Tracker(void * pvParameters);
char backSpace = 0;
char clear = 0;
void LED_Blinker(void * pvParameters);
int main(void)
{
    /* Replace with your application code */
	lcd_init();
	DDRD = 0x0F;
	DDRA = 0x00;
	TCCR0 |= (1 << WGM00)|(1 << WGM01)|(1 << COM01)|(1 << CS00);
	DDRB |= (1 << PB3);
	OCR0 = 0;
	xTaskCreate( Welcome, "Welcome_Name", 100, NULL, 2, &handle_task1);

/* Start the scheduler. */
	vTaskStartScheduler();

}

void vApplicationIdleHook( void )
{
}


void Welcome(void * pvParameters){
	display_welcome();
	xTaskCreate( KEYPAD_Scanner, "KEYPAD_Scanner_Name", 100, NULL, 2, &handle_task2);
	waiting_for_key();
	xTaskCreate( LED_Blinker, "LED_Blinker_Name", 100, NULL, 2, &handle_LED_Blinker);
	while(1);
	vTaskDelete(handle_task1);

	
}

void LED_Blinker(void * pvParameters)
{
	//PORTB |= (1 << PB3);
	vTaskDelete(handle_task1);
	static int blinkCounter = 0;
	int a = xTaskGetTickCount();
	while(1)
	{
		vTaskDelayUntil(&a,1);
		blinkCounter++;
		if(blinkCounter >= 1000)
		{
			blinkCounter = 0;
		}
		if(blinkCounter/750)
		{
			PORTB &= ~(1 << PB3);
		}
		else if(blinkCounter/500)
		{
			OCR0 = ((750 - blinkCounter) * 0.4);		
		}
		else if(blinkCounter/250)
		{
			PORTB |= (1 << PB3);
		}
		else
		{
			OCR0 = (blinkCounter * 0.4);
		}
	}
}
void KEYPAD_Scanner(void * pvParameters){
	int y = xTaskGetTickCount();
	while(1){
		vTaskDelayUntil(&y,20);
		KEYPAD_Scan();
		if(pressed_number != 0)
		{
			lcd_clrScreen();
			xTaskCreate( Calculator, "Calculator_Name", 100, NULL, 3, &handle_Calculator);
			vTaskDelete(handle_task1);
			vTaskDelete(handle_LED_Blinker);
		}

	}
	
}
void Calculator(void * pvParameters)
{
		xTaskCreate( BUTTON_Tracker, "BUTTON_Tracker_Name", 100, NULL, 2, &handle_BUTTON_Tracker);

		static char resultStr[16];
		static char num1Str[16];
		static char num2Str[16];
		//int x = xTaskGetTickCount();
		//xTaskCreate( BUTTON_Tracker, "BUTTON_Tracker_Name", 100, NULL, 4, &handle_BUTTON_Tracker);
		while(1)
		{
			//vTaskDelayUntil(&x,20);
			KEYPAD_Scan();
			_delay_ms(20);
			if(pressed_number == 'C')
			{
				lcd_clrScreen();
				lcd_displayChar('0');
				num1 = 0;
				num2 = 0;
				result = 0;
				continue;
			}
			if((pressed_number != 0) && isdigit(pressed_number))
				{
					if(CALC1 == Calculate)
					{
						lcd_clrScreen();
						CALC1 = GetNUM1;
						num1 = 0;
						num2 = 0;
					}
					lcd_displayChar(pressed_number);
					if(CALC1 == GetNUM1){
					num1 *= 10;
					num1 += pressed_number - '0';
					}
					else if(CALC1 == GetNUM2)
					{
						num2 *= 10;
						num2 += pressed_number - '0';
					}
				}
			else if((pressed_number != 0) && (!isdigit(pressed_number)) && (pressed_number != '='))
			{
				lcd_clrScreen();
				if(CALC1 == Calculate)
				{
					num1 = result;
					num2 = 0;
				}
				switch(pressed_number)
				{
					case '+':
						operation = ADD;
						//lcd_displayChar('+');
						break;
					case '-':
						operation = SUB;
						//lcd_displayChar('-');
						break;
					case '*':
						operation = MUL;
						//lcd_displayChar('x');
						break;
					case '%':
						operation = DIV;
						//lcd_displayChar('/');
						break;
				}
				CALC1 = GetNUM2;
			}	
			else if(pressed_number == '=')
			{
				lcd_clrScreen();
				CALC1 = Calculate;
				switch(operation)
				{
					case ADD:
					result = num1 + num2;
					break;
					case SUB:
					result = num1 - num2;
					break;
					case MUL:
					result = num1 * num2;
					break;
					case DIV:
					result = num1 / num2;
					break;
				}
				itoa(result,resultStr,10);
				lcd_dispString(resultStr);
				lcd_gotoxy(0,0);
			}
		}
}
void BUTTON_Tracker(void * pvParameters)
{
	char counterStr[16];
	int z = xTaskGetTickCount();
	static int counter= 0;
	while(1)
	{
		vTaskDelayUntil(&z,20);
		if((PINA&(1<<0)) == 0)
		{
			counter++;
			//itoa(counter,counterStr,10);
			//lcd_dispString("Here!");
		}
		else
		{
			counter = 0;
			backSpace = 0;
			clear = 0;
		}
		if(counter >= 150)
		{
			clear = 1;
			lcd_clrScreen();
			num2 = 0;
			num1 = 0;
			result = 0;
		}
		else
		{
			backSpace = 1;
			if(CALC1 == GetNUM1)
			{
				num1 /= 10;
			}
			else if(CALC1 == GetNUM2)
			{
				num2 /= 10;
			}	
			else if(CALC1 == Calculate)
			{
				result /= 10;
			}		
			//lcd_dispString(counterStr);
			
		}
	}
	//lcd_displayChar(clear+'0');
}
void display_welcome()
{
  for(char k = 0 ; k < 3 ; k++){
	for(char j = 0 ; j < 15-strlen("WELCOME!") ; j++)
	{
		lcd_clrScreen();
		for(char i = 0 ; i < j ; i++)
		{
			lcd_dispString(" ");
		}
		lcd_dispString("WELCOME!");	
	
		_delay_ms(500/(15-strlen("WELCOME!")));
	}
	for(char j = 0 ; j < 15-strlen("WELCOME!") ; j++)
	{
		lcd_clrScreen();
		for(char i = 15-strlen("WELCOME!") ; i > j ; i--)
		{
			lcd_dispString(" ");
		}
		lcd_dispString("WELCOME!");
		_delay_ms(500/(15-strlen("WELCOME!")));
	}
}
		lcd_clrScreen();

}

void waiting_for_key()
{
	for(char i = 0 ; i < 10000/750 ; i++)
	{
	lcd_gotoxy(0,0);
	lcd_dispString("Press any key to");
	lcd_gotoxy(1,4);
	lcd_dispString("Continue");
	_delay_ms(500);
	lcd_clrScreen();
	_delay_ms(250);	
	}
}

void KEYPAD_Scan(void){
		static char flag = 0;
		for(char i = 0 ; i < 4 ; i++)
		{
			PORTD = (1 << i);
			switch(PIND)
			{
				case 0x11:
				pressed_number = ('+');
				break;
				case 0x21:
				pressed_number = ('-');
				break;
				case 0x41:
				pressed_number = ('*');
				break;
				case 0x81:
				pressed_number = ('%');
				break;
				
				case 0x12:
				pressed_number = ('=');
				break;
				case 0x22:
				pressed_number = ('3');
				break;
				case 0x42:
				pressed_number = ('6');
				break;
				case 0x82:
				pressed_number = ('9');
				break;
				
				case 0x14:
				pressed_number = ('0');
				break;
				case 0x24:
				pressed_number = ('2');
				break;
				case 0x44:
				pressed_number = ('5');
				break;
				case 0x84:
				pressed_number = ('8');
				break;
				
				case 0x18:
				pressed_number = ('C');
				break;
				case 0x28:
				pressed_number = ('1');
				break;
				case 0x48:
				pressed_number = ('4');
				break;
				case 0x88:
				pressed_number = ('7');
				break;
				default:
				pressed_number = 0;
				break;
			}
			if(pressed_number != 0)
			{
				flag = (flag | (0xFF)) - flag ;
				pressed_number &= flag;
				break;
			}




		}
		
		if((!pressed_number) && (flag))
		{
			flag = 0;
		}
}

