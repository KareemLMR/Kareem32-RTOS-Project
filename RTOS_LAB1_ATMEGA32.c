#define F_CPU 8000000

#include "avr/io.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "util/delay.h"
#include "lcd_4bit.h"

/* Define Tasks Priorities */
#define  TASK1_PRIORITY (3)
#define  TASK2_PRIORITY (2)

void vApplicationIdleHook( void );

void Task1(void * pvParameters);
void Task2(void * pvParameters);
void LCD_DisplayPattern(void);
void KEYPAD_Scan(void);
void LCD_Display_KP(void);

xTaskHandle handle_task1;	
xTaskHandle handle_task2;
//xSemaphoreHandle sem;

int x = 0;
char pressed_number = 0;

int main( void )
{
	lcd_init();
	DDRA = 0xFF;
	
	DDRD = 0x0F;
	PORTD = 0xFF; //Pull up resistors for inputs and Set outputs to High
	
	//vSemaphoreCreateBinary(sem);
	
	/* Create at least one task, in this case the task function defined above is
	created. Calling vTaskStartScheduler() before any tasks have been created
	will cause the idle task to enter the Running state. */
	char xQueue;
	xQueue = xQueueCreate(16,1);

	xTaskCreate( Task1, "Task1_Name", 100, NULL, 2, &handle_task1);
	xTaskCreate( Task2, "Task2_Name", 100, NULL, 1, &handle_task2);
		
	
	/* Start the scheduler. */
	
	vTaskStartScheduler();
	/* This code will only be reached if the idle task could not be created inside
	vTaskStartScheduler(). An infinite loop is used to assist debugging by
	ensuring this scenario does not result in main() exiting. */
	
	
	return 0;
}

void vApplicationIdleHook( void ){
	if(x++ == 1000){
		PORTA ^= 0xFF;
		_delay_ms(500);	
		x = 0;
	}
}


void Task1(void * pvParameters){
	
	while(1){
  
		
		KEYPAD_Scan();
	}
	
}

void Task2(void * pvParameters){

	while(1){
		LCD_Display_KP();
	}
	
}


//Warning: This can not handle 2 buttons pressed at the same time (It is safe in simulation due to the existence of 1 mouse)
void KEYPAD_Scan(void){
	char i = 0, j = 0, flag = 0, new = 0;
	static char old = 0;
	static char* ptrPressed;
	for(i = 1; i < 4; i++){
		PORTD |= 0x0F;
		PORTD &= ~(1<<i);
		for(j = 1; j < 4; j++){
			if( ( ~PIND & (1<<(j+4)) ) == (1<<(j+4)) ){ //For buttons : 1->9
				new = (j-1)*3 + i + 48;
				flag = 1;
				break;
			}else if(( ~PIND & (1<<4) ) == (1<<4) && i == 2){ //For buttons : 0
				new = '0';
				flag = 1;
				break;
			}
		}
		if(flag == 1) break;
	}
	
	if(flag == 1){ //Button Pressed
		if(old != new){
			old = new;
		}else{
			new = 0;
		}	
	}else{ //Button Released
		old = 0;
		new = 0;
	}
	
	pressed_number = new;
	ptrPressed = &pressed_number;
	if(xQueueSend( xQueue,ptrPressed, 100))
	{
		
	}
}

void LCD_Display_KP(void){
	static char recieved;
	static char* ptr = &recieved;
	if(xQueueReceive(xQueue,ptr, 20)){
	if(pressed_number != 0){
		lcd_clrScreen();
		lcd_displayChar(recieved);
		}  
		else{
		//Do Nothing
	}
	
	
	}
	}



