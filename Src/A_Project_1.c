/******************************************************************************
  * @file  FND.c
  * @author  Huins Embbeded Team  by HyunKH
  * @version V1.0.0
  * @date    2015. 02. 10
  * @brief   FND Control (StopWatch)
  ******************************************************************************/

/* Includes ------------------------------------------------------------------*/           
#include "lpc17xx_timer.h"
#include "LPC1768_utility.h"
#include "LPC1768_motor.h"
#include "lpc17xx_adc.h"

#define _ADC_INT         ADC_ADINTEN2
#define _ADC_CHANNEL      ADC_CHANNEL_2
/* Private variables ---------------------------------------------------------*/
uint8_t time_10h =0 ;
uint8_t time_1h =0 ;
uint8_t time_10m = 0;
uint8_t time_1m = 0;
uint8_t time_1s = 0;

uint8_t key = 0;
uint32_t circle = 150; 
uint16_t led=0;
uint8_t Int_flag = 0;

uint8_t mode1=1,mode2=1,mode3=1, timer_mode = 0, a_mode = 0;
uint8_t tmp = 0, delay_time=0, pre_tmp = 0; 
__IO uint32_t adc_value;

void circulation(uint8_t keypad);
void Timer() {}
void TIMER0_Config(void);

void DELAY(unsigned long d_t)
{
  long k;
  for(k=0; k<d_t; k++);
}
//Keypad
uint8_t Keypad_Col_Num() {
	uint8_t col_num = 0;
	
	LPC_GPIO1->FIOPIN &= ~(1<<21);
	LPC_GPIO1->FIOPIN |= (1<<21);
	LPC_GPIO1->FIOPIN &= ~(1<<21);
	if((LPC_GPIO0->FIOPIN >> 23) & 0x01);
	else col_num = 4;
	if((LPC_GPIO0->FIOPIN >> 24) & 0x01);
	else col_num = 3;
	if((LPC_GPIO3->FIOPIN >> 25) & 0x01);
	else col_num = 2;
	if((LPC_GPIO3->FIOPIN >> 26) & 0x01);
	else col_num = 1; 
	LPC_GPIO1->FIOPIN |= (1<<21);
	
	return col_num;
}	
uint8_t Keypad() {
	
	uint8_t fio_backup = LPC_GPIO0->FIOPIN;
	uint8_t key_tmp = 0,key_result = 0;
	//EXT_IO_'C'
	LPC_GPIO0->FIOPIN |= (1<<19); 	
	LPC_GPIO0->FIOPIN |= (1<<20); 	
	LPC_GPIO0->FIOPIN &= ~(1<<21); 
	
	
	//Clock activating code
	LPC_GPIO0->FIOPIN |= (1<<4);
	//Pin mode change code
	LPC_GPIO0->FIODIR &= ~(1<<23);
	LPC_GPIO0->FIODIR &= ~(1<<24);
	LPC_GPIO3->FIODIR &= ~(1<<25);
	LPC_GPIO3->FIODIR &= ~(1<<26);
	
	//set keypad_row 1
	LPC_GPIO0->FIOPIN &= ~(1<<5);
	LPC_GPIO0->FIOPIN |= (1<<10);
	LPC_GPIO2->FIOPIN |= (1<<12);
	LPC_GPIO2->FIOPIN |= (1<<13);
	LPC_GPIO2->FIOPIN |= (1<<11);
	LPC_GPIO2->FIOPIN &= ~(1<<11);
	
	key_tmp = Keypad_Col_Num();
	if(key_tmp != 0) key_result=key_tmp;
	
	//set keypad_row 2
	LPC_GPIO0->FIOPIN |= (1<<5);
	LPC_GPIO0->FIOPIN &= ~(1<<10);
	LPC_GPIO2->FIOPIN |= (1<<12);
	LPC_GPIO2->FIOPIN |= (1<<13);
	LPC_GPIO2->FIOPIN |= (1<<11);
	LPC_GPIO2->FIOPIN &= ~(1<<11);
	
	
	key_tmp = Keypad_Col_Num();
	if(key_tmp != 0) key_result=key_tmp+4;
	
	//set keypad_row 3
	LPC_GPIO0->FIOPIN |= (1<<5);
	LPC_GPIO0->FIOPIN |= (1<<10);
	LPC_GPIO2->FIOPIN &= ~(1<<12);
	LPC_GPIO2->FIOPIN |= (1<<13);
	LPC_GPIO2->FIOPIN |= (1<<11);
	LPC_GPIO2->FIOPIN &= ~(1<<11);
	
	key_tmp = Keypad_Col_Num();
	if(key_tmp != 0) key_result=key_tmp+8;
	
	//set keypad_row 4
	LPC_GPIO0->FIOPIN |= (1<<5);
	LPC_GPIO0->FIOPIN |= (1<<10);
	LPC_GPIO2->FIOPIN |= (1<<12);
	LPC_GPIO2->FIOPIN &= ~(1<<13);
	LPC_GPIO2->FIOPIN |= (1<<11);
	LPC_GPIO2->FIOPIN &= ~(1<<11);
	
	key_tmp = Keypad_Col_Num();
	if(key_tmp != 0) key_result=key_tmp+12;
	
	//Clock deactivating code
	LPC_GPIO0->FIOPIN = fio_backup;
	//Pin mode change code
	LPC_GPIO0->FIODIR |= (1<<5);
	LPC_GPIO0->FIODIR |= (1<<10);
  LPC_GPIO2->FIODIR |= (1<<12);
  LPC_GPIO2->FIODIR |= (1<<13);
	return key_result;
}
void Keypad_input(uint8_t value) {
	switch(value) {
		case 1:
			mode1 = 1;
			break;
		case 2:
			mode1 = 2;
			break;
		case 3:
			mode1 = 3;
			break;
		case 5:
			mode2 = 1;
			a_mode= 0;
			break;
		case 6:
			mode2 = 2;
			a_mode=0;
			break;
		case 7:
			if(a_mode != 1) delay_time = 10;
			a_mode = 1;
			break;
		case 9:
			mode3 = 1;
		  circulation(mode3);
			break;
		case 10:
			mode3 = 2;
		  circulation(mode3);
		  break;
	}
}

//FND
void FND(uint8_t fnd_select, uint8_t fnd_data) {
	//FND_init
	uint8_t fio_backup = LPC_GPIO0->FIOPIN;
	LPC_GPIO0->FIODIR |= (1<<4)|(1<<5)|(1<<10)|(1<<11)|(1<<19)|(1<<20)|(1<<21)|(1<<23)|(1<<24); 	
	LPC_GPIO1->FIODIR |= (1<<21);
	LPC_GPIO2->FIODIR |= (1<<11)|(1<<12)|(1<<13);
	LPC_GPIO3->FIODIR |= (1<<25)|(1<<26);
	
	//FND Select
	LPC_GPIO0->FIOPIN &= ~(1<<23);	//	COM1
	LPC_GPIO0->FIOPIN &= ~(1<<24);	//	COM2
	LPC_GPIO1->FIOPIN &= ~(1<<21);	//	COM3
	LPC_GPIO2->FIOPIN &= ~(1<<11);	//	COM4
	LPC_GPIO2->FIOPIN &= ~(1<<12);	//	COM5
	LPC_GPIO2->FIOPIN &= ~(1<<13);	//	COM6
	LPC_GPIO3->FIOPIN &= ~(1<<25);	//	COM7
	LPC_GPIO3->FIOPIN &= ~(1<<26);	//	COM8
	switch(fnd_select){
		case 1:
			LPC_GPIO0->FIOPIN |= (1<<23);
			break;
		case 2:
			LPC_GPIO0->FIOPIN |= (1<<24);
			break;
		case 3:
			LPC_GPIO1->FIOPIN |= (1<<21);
			break;
		case 4:
			LPC_GPIO2->FIOPIN |= (1<<11);
			break;
		case 5:
			LPC_GPIO2->FIOPIN |= (1<<12);
			break;
		case 6:
			LPC_GPIO2->FIOPIN |= (1<<13);
			break;
		case 7:
			LPC_GPIO3->FIOPIN |= (1<<25);
			break;
		case 8:
			LPC_GPIO3->FIOPIN |= (1<<26);
			break;
	}
	LPC_GPIO0->FIOPIN |= (1<<10);
	LPC_GPIO0->FIOPIN &= ~(1<<10);
	
	//FND Data
	LPC_GPIO0->FIOPIN &= ~(1<<23);	//	A
	LPC_GPIO0->FIOPIN &= ~(1<<24);	//	B
	LPC_GPIO1->FIOPIN &= ~(1<<21);	//	C
	LPC_GPIO2->FIOPIN &= ~(1<<11);	//	D
	LPC_GPIO2->FIOPIN &= ~(1<<12);	//	E
	LPC_GPIO2->FIOPIN &= ~(1<<13);	//	F
	LPC_GPIO3->FIOPIN &= ~(1<<25);	//	G
	LPC_GPIO3->FIOPIN |= (1<<26);		//	DP
	switch(fnd_data) {
		case 0: 
			LPC_GPIO3->FIOPIN |= (1<<25);
			break;
		case 1: 
			LPC_GPIO0->FIOPIN |= (1<<23);
			LPC_GPIO2->FIOPIN |= (1<<11);
			LPC_GPIO2->FIOPIN |= (1<<12);
			LPC_GPIO2->FIOPIN |= (1<<13);
			LPC_GPIO3->FIOPIN |= (1<<25);
			break;
		case 2: 
			LPC_GPIO1->FIOPIN |= (1<<21);
			LPC_GPIO2->FIOPIN |= (1<<13);
			break;
		case 3: 
			LPC_GPIO2->FIOPIN |= (1<<12);
			LPC_GPIO2->FIOPIN |= (1<<13);
			break;
		case 4: 
			LPC_GPIO0->FIOPIN |= (1<<23);
			LPC_GPIO2->FIOPIN |= (1<<11);
			LPC_GPIO2->FIOPIN |= (1<<12);
			break;
		case 5: 
			LPC_GPIO0->FIOPIN |= (1<<24);
			LPC_GPIO2->FIOPIN |= (1<<12);
			break;
		case 6: 
			LPC_GPIO0->FIOPIN |= (1<<24);
			break;
		case 7: 
			LPC_GPIO2->FIOPIN |= (1<<11);
			LPC_GPIO2->FIOPIN |= (1<<12);
			LPC_GPIO2->FIOPIN |= (1<<13);
			LPC_GPIO3->FIOPIN |= (1<<25);
			break;
		case 8: 
			break;
		case 9: 
			LPC_GPIO2->FIOPIN |= (1<<11);
			LPC_GPIO2->FIOPIN |= (1<<12);
			break;			
	}
	LPC_GPIO0->FIOPIN |= (1<<5);
	LPC_GPIO0->FIOPIN &= ~(1<<5);	
	
	//Recover with backuped fio
	LPC_GPIO0->FIOPIN = fio_backup;
}
void FND_Dir(uint8_t mode1_value) {
	//FND_init
	uint8_t fio_backup = LPC_GPIO0->FIOPIN;
	LPC_GPIO0->FIODIR |= (1<<4)|(1<<5)|(1<<10)|(1<<11)|(1<<19)|(1<<20)|(1<<21)|(1<<23)|(1<<24); 	
	LPC_GPIO1->FIODIR |= (1<<21);
	LPC_GPIO2->FIODIR |= (1<<11)|(1<<12)|(1<<13);
	LPC_GPIO3->FIODIR |= (1<<25)|(1<<26);
	
	//FND Select 1
	LPC_GPIO0->FIOPIN |= (1<<23);	//	COM1
	LPC_GPIO0->FIOPIN &= ~(1<<24);	//	COM2
	LPC_GPIO1->FIOPIN &= ~(1<<21);	//	COM3
	LPC_GPIO2->FIOPIN &= ~(1<<11);	//	COM4
	LPC_GPIO2->FIOPIN &= ~(1<<12);	//	COM5
	LPC_GPIO2->FIOPIN &= ~(1<<13);	//	COM6
	LPC_GPIO3->FIOPIN &= ~(1<<25);	//	COM7
	LPC_GPIO3->FIOPIN &= ~(1<<26);	//	COM8
	
	//FND Select CS
	LPC_GPIO0->FIOPIN |= (1<<10);
	LPC_GPIO0->FIOPIN &= ~(1<<10);
	
	//FND_Data Reset
	LPC_GPIO0->FIOPIN |= (1<<23);		//	A
	LPC_GPIO0->FIOPIN |= (1<<24);		//	B
	LPC_GPIO1->FIOPIN |= (1<<21);		//	C
	LPC_GPIO2->FIOPIN |= (1<<11);		//	D
	LPC_GPIO2->FIOPIN |= (1<<12);		//	E
	LPC_GPIO2->FIOPIN |= (1<<13);		//	F
	LPC_GPIO3->FIOPIN |= (1<<25);		//	G
	LPC_GPIO3->FIOPIN |= (1<<26);		//	DP
	
	//FND Data
	switch(mode1_value) {
			case 1: 
				LPC_GPIO0->FIOPIN &= ~(1<<23);	//	A
				break;
			case 2: 
				LPC_GPIO3->FIOPIN &= ~(1<<25);	//	G
				break;
			case 3: 
				LPC_GPIO2->FIOPIN &= ~(1<<11);	//	D
				break;
		}
	
	LPC_GPIO0->FIOPIN |= (1<<5);
	LPC_GPIO0->FIOPIN &= ~(1<<5);

	//Recover with backuped fio
	LPC_GPIO0->FIOPIN = fio_backup;		
}
	

void FND_Mode(uint8_t mode2_value) {
	//FND_init
	uint8_t fio_backup = LPC_GPIO0->FIOPIN;
	LPC_GPIO0->FIODIR |= (1<<4)|(1<<5)|(1<<10)|(1<<11)|(1<<19)|(1<<20)|(1<<21)|(1<<23)|(1<<24); 	
	LPC_GPIO1->FIODIR |= (1<<21);
	LPC_GPIO2->FIODIR |= (1<<11)|(1<<12)|(1<<13);
	LPC_GPIO3->FIODIR |= (1<<25)|(1<<26);
	
	//FND Select 1
	LPC_GPIO0->FIOPIN &= ~(1<<23);	//	COM1
	LPC_GPIO0->FIOPIN |= (1<<24);	//	COM2
	LPC_GPIO1->FIOPIN &= ~(1<<21);	//	COM3
	LPC_GPIO2->FIOPIN &= ~(1<<11);	//	COM4
	LPC_GPIO2->FIOPIN &= ~(1<<12);	//	COM5
	LPC_GPIO2->FIOPIN &= ~(1<<13);	//	COM6
	LPC_GPIO3->FIOPIN &= ~(1<<25);	//	COM7
	LPC_GPIO3->FIOPIN &= ~(1<<26);	//	COM8
	
	//FND Select CS
	LPC_GPIO0->FIOPIN |= (1<<10);
	LPC_GPIO0->FIOPIN &= ~(1<<10);
	
	//FND_Data Reset
	LPC_GPIO0->FIOPIN &= ~(1<<23);	//	A
	LPC_GPIO0->FIOPIN &= ~(1<<24);	//	B
	LPC_GPIO1->FIOPIN &= ~(1<<21);	//	C
	LPC_GPIO2->FIOPIN &= ~(1<<11);	//	D
	LPC_GPIO2->FIOPIN &= ~(1<<12);	//	E
	LPC_GPIO2->FIOPIN &= ~(1<<13);	//	F
	LPC_GPIO3->FIOPIN &= ~(1<<25);	//	G
	LPC_GPIO3->FIOPIN &= ~(1<<26);	//	DP
	
	//FND Data
	switch(mode2_value) {
			case 1: 
				LPC_GPIO2->FIOPIN |= (1<<11);	//	D
				break;
			case 2: 
				LPC_GPIO0->FIOPIN |= (1<<23);	//	A
				LPC_GPIO2->FIOPIN |= (1<<11);	//	D
				break;
		}
	
	LPC_GPIO0->FIOPIN |= (1<<5);
	LPC_GPIO0->FIOPIN &= ~(1<<5);

	//Recover with backuped fio
	LPC_GPIO0->FIOPIN = fio_backup;		
}
	

void FND_Display() {
	FND(8,time_1m);
	Delay(5000);
	FND(7,time_10m);
	Delay(5000);
	FND(6,time_1h);
	Delay(5000);
	FND(5,time_10h);
	Delay(5000);
	if(a_mode == 1) {
		tmp = ((adc_value/285))+18;;
	}else if(mode2==1) {
		tmp = ((adc_value/500)%10)+18;
	}else if(mode2 == 2){
		tmp = ((adc_value/600)%10)+26;
	}
	if(tmp != pre_tmp) delay_time=10;
	pre_tmp = tmp;
	FND(4,(tmp)%10);
	Delay(10000);
	FND(3,(tmp/10)%10);
	Delay(10000);
	Delay(5000);
	FND_Mode(mode2);
	Delay(5000);
	FND_Dir(mode1);
	Delay(5000);
}

//Adc
void ADC_IRQHandler(void)
{
  adc_value = 0;
  if (ADC_ChannelGetStatus(LPC_ADC,_ADC_CHANNEL,ADC_DATA_DONE))
  {
     adc_value =  ADC_ChannelGetData(LPC_ADC,_ADC_CHANNEL);
     NVIC_DisableIRQ(ADC_IRQn);
  }
}
//LED
void LED_Init(uint8_t Led)
{
	switch(Led)
	{
		case LED1:
			LPC_GPIO1->FIODIR |= (1<<28);
			LPC_GPIO1->FIOCLR = (1<<28);
			break;
		
		case LED2:
			LPC_GPIO1->FIODIR |= (1<<29);
			LPC_GPIO1->FIOCLR = (1<<29);
			break;

		case LED3:
			LPC_GPIO1->FIODIR |= (1<<31);
			LPC_GPIO1->FIOCLR = (1<<31);
			break;

		case LED4:
			LPC_GPIO2->FIODIR |= (1<<2);
			LPC_GPIO2->FIOCLR = (1<<2);
			break;

		case LED5:
			LPC_GPIO2->FIODIR |= (1<<3);
			LPC_GPIO2->FIOCLR = (1<<3);
			break;
		
		case LED6:
			LPC_GPIO2->FIODIR |= (1<<4);
			LPC_GPIO2->FIOCLR = (1<<4);
			break;

		case LED7:
			LPC_GPIO2->FIODIR |= (1<<5);
			LPC_GPIO2->FIOCLR = (1<<5);
			break;

		case LED8:
			LPC_GPIO2->FIODIR |= (1<<6);
			LPC_GPIO2->FIOCLR = (1<<6);
			break;
		
		case LED_ALL:	
			LPC_GPIO1->FIODIR |= (1<<28)|(1<<29)|(1<<31);
			LPC_GPIO2->FIODIR |= (1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6);
				
			break;
	}
}

void LED_On(uint8_t Led)
{
	switch(Led)
	{
		case LED1 :
			LPC_GPIO1->FIOSET = (1<<28);
			break;
		
		case LED2 :
			LPC_GPIO1->FIOSET = (1<<29);
			break;
		
		case LED3 :
			LPC_GPIO1->FIOSET = (1<<31);
			break;
		
		case LED4  :
			LPC_GPIO2->FIOSET = (1<<2);
			break;
		
		case LED5 :
			LPC_GPIO2->FIOSET = (1<<3);
			break;
		
		case LED6 :
			LPC_GPIO2->FIOSET = (1<<4);
			break;
		
		case LED7 :
			LPC_GPIO2->FIOSET = (1<<5);
			break;
		
		case LED8 :
			LPC_GPIO2->FIOSET = (1<<6);
			break;	
		
		case LED_ALL  :
			LPC_GPIO1->FIODIR |= (1<<28)|(1<<29)|(1<<31);
			LPC_GPIO2->FIODIR |= (1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6);	
			break;			
	}
}

void LED_Off(uint8_t Led)
{
	switch(Led)
	{
		case 1 :
			LPC_GPIO1->FIOCLR = (1<<28);
			break;
		
		case 2 :
			LPC_GPIO1->FIOCLR = (1<<29);
			break;
		
		case 3 :
			LPC_GPIO1->FIOCLR = (1<<31);
			break;
		
		case LED4  :
			LPC_GPIO2->FIOCLR = (1<<2);
			break;
		
		case LED5 :
			LPC_GPIO2->FIOCLR = (1<<3);
			break;
		
		case LED6 :
			LPC_GPIO2->FIOCLR = (1<<4);
			break;
		
		case LED7 :
			LPC_GPIO2->FIOCLR = (1<<5);
			break;
		
		case LED8 :
			LPC_GPIO2->FIOCLR = (1<<6);
			break;		
		case LED_ALL :
         LPC_GPIO1->FIOCLR = (1<<28)|(1<<29)|(1<<31);
         LPC_GPIO2->FIOCLR = (1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6);   
         break;
	}
}


void circulation(uint8_t keypad){
  LED_Off(LED7); LED_Off(LED8);
  if(keypad == 1){
     LED_On(LED7);
  }else{
     LED_On(LED8);
  }
}


//StepMotor
void stepmotor_led_joystick(uint8_t joystick){
	if(joystick == 3){
		circle += 30;
		led++;
		if(led >= 5 || circle >= 310){
			led = 5;
			circle = 310;
		}
		LED_On(led);
	}else if(joystick == 5){
		circle -= 30;
		LED_Off(led);
		led--;
		if(led <=0 || circle <= 160){
			led = 0;
			circle = 160;
		}
	}
}
void Stepmotor_Init(void){
	LPC_GPIO0->FIODIR |= (1<<4)|(1<<5)|(1<<10)|(1<<11)|(1<<19)|(1<<20)|(1<<21)|(1<<23)|(1<<24); 	
	LPC_GPIO1->FIODIR |= (1<<21);
	LPC_GPIO2->FIODIR |= (1<<11)|(1<<12)|(1<<13);
	LPC_GPIO3->FIODIR |= (1<<25)|(1<<26);
}

void StepMotor_Cycle_Velocity(uint8_t cycle, uint32_t velocity)
{
  uint32_t count = 0;
	uint8_t fiopin = LPC_GPIO0->FIOPIN;
	Stepmotor_Init();
  for(count = 0; count < cycle * 24; count++)
  {
    LPC_GPIO0->FIOSET = (1<<5);
    LPC_GPIO0->FIOCLR = (1<<10);
    LPC_GPIO0->FIOSET = (1<<23);
    LPC_GPIO0->FIOCLR = (1<<24);
    DELAY(SEC_1/velocity);
   
		LPC_GPIO0->FIOCLR = (1<<5);
    LPC_GPIO0->FIOSET = (1<<10);
    LPC_GPIO0->FIOSET = (1<<23);
    LPC_GPIO0->FIOCLR = (1<<24);
    DELAY(SEC_1/velocity);
      
    LPC_GPIO0->FIOCLR = (1<<5);
    LPC_GPIO0->FIOSET = (1<<10);
    LPC_GPIO0->FIOCLR = (1<<23);
    LPC_GPIO0->FIOSET = (1<<24);
    DELAY(SEC_1/velocity);
      
    LPC_GPIO0->FIOSET = (1<<5);
    LPC_GPIO0->FIOCLR = (1<<10);
    LPC_GPIO0->FIOCLR = (1<<23);
    LPC_GPIO0->FIOSET = (1<<24);
    DELAY(SEC_1/velocity);
  }
	LPC_GPIO0->FIOPIN = fiopin;
}

      
//Joystick
void Joystick_Init(void){
   LPC_GPIO4 ->FIODIR |= (1<<28);
	
   LPC_GPIO1 ->FIODIR &= ~(1<<20);
   LPC_GPIO1 ->FIODIR &= ~(1<<23);
   LPC_GPIO1 ->FIODIR &= ~(1<<24);
   LPC_GPIO1 ->FIODIR &= ~(1<<25);
   LPC_GPIO1 ->FIODIR &= ~(1<<26);
}
uint8_t Joystick_read(){
  uint8_t js_val = 0;
   
  if((LPC_GPIO1 ->FIOPIN >> 20) & 0x01);
  else js_val = 1;
  if((LPC_GPIO1 ->FIOPIN >> 23) & 0x01); 
  else js_val = 2;
  if((LPC_GPIO1 ->FIOPIN >> 24) & 0x01);
	else js_val = 3;
  if((LPC_GPIO1 ->FIOPIN >> 25) & 0x01);
  else js_val = 4;
  if((LPC_GPIO1 ->FIOPIN >> 26) & 0x01);
  else js_val = 5;
  
  return js_val;
}



//Interrupt Control
void EINT0_IRQHandler(void)
{
	EXTI_ClearEXTIFlag(EXTI_EINT0);
	if(Int_flag == 0) Int_flag = 1;
	else Int_flag =0;
}

void Time_Set() {
	key=Keypad();
	time_1s=0;
	if(key == 8)time_1m++;
	if(key == 7)time_10m++;
	if(key == 6)time_1h++;
	if(key == 12){ 
		if(time_1m == 0){
			if(time_10m == 0){
				time_10m=6;
			}
			time_10m--;
			time_1m = 10;
		}
		time_1m--;
	}
	if(key == 11){
		if(time_10m == 0){
			time_10m = 6;
		}
		time_10m--;
	}
	if(key == 10){
		if(time_10h == 0 && time_1h == 0){
			time_10h =2;
			time_1h =4;
		}
		time_1h--;
	}
	if(time_1m == 10)
	{
		time_10m++;
		time_1m =0;
	}
	if(time_10m == 6)
	{																																								
		time_10m = 0;
	}	
	if(time_1h == 10){
		time_10h++;
		time_1h =0;
	}		
	if(time_10h ==2 && time_1h ==4){
		time_10h =0;
		time_1h =0;
	}
}

//Auto Mode Control
void Auto_Mode() {
	uint16_t set_speed = 0;
	if(tmp == 32 || tmp == 18) set_speed = 5; 
	if(tmp == 31 || tmp == 19) set_speed = 4;
	if(tmp == 30 || tmp == 29 || tmp == 21 || tmp == 20) set_speed = 3;
	if(tmp == 28 || tmp == 23 || tmp ==22) set_speed = 2;
	if(tmp == 27 || tmp == 24) set_speed = 1;
	if(tmp == 26 || tmp == 25) set_speed = 0;
	if(delay_time == 0) {
			if(led > set_speed) stepmotor_led_joystick(5);
			else if(led < set_speed) stepmotor_led_joystick(3);
	}
}

//Main Function
TIM_TIMERCFG_Type TIM_ConfigStruct;
TIM_MATCHCFG_Type TIM_MatchConfigStruct ;

int main (void) 
{
	PINSEL_CFG_Type PinCfg;
	
	//Hardware init
	SystemInit();
	Stepmotor_Init();
	Led_Init(LED_ALL);
	Joystick_Init();
	
	//Interrupts init
	EXTI_Init();	
	AF_ConfigPin(GPIO_PORT_2, PINSEL_PIN_10, PINSEL_FUNC_1);	
	EXTI_ConfigPin(EXTI_EINT0); 
	NVIC_EnableIRQ(EINT0_IRQn); 
	
	TIMER0_Config();
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);	
	TIM_ConfigMatch(LPC_TIM0,&TIM_MatchConfigStruct);		
	NVIC_EnableIRQ(TIMER0_IRQn);	
	TIM_Cmd(LPC_TIM0, ENABLE);	
	
   
  PinCfg.Funcnum = 1;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Pinnum = 25;
  PinCfg.Portnum = 0;
  PINSEL_ConfigPin(&PinCfg);
  
  ADC_Init(LPC_ADC, 200000);
  ADC_IntConfig(LPC_ADC,_ADC_INT,ENABLE);
  ADC_ChannelCmd(LPC_ADC,_ADC_CHANNEL,ENABLE);
   
  NVIC_SetPriority(ADC_IRQn, ((0x01<<3)|0x01));
	
  while(1) 
	{
		ADC_StartCmd(LPC_ADC,ADC_START_NOW);
    NVIC_EnableIRQ(ADC_IRQn);
		if(Int_flag ==1){
			StepMotor_Cycle_Velocity(1, circle);
		}
		else {
			uint8_t value = 0;
			FND_Display();
			value = Keypad();
			if(timer_mode == 0)
				Keypad_input(value);
			if(value == 16) {
				if(timer_mode == 1) timer_mode = 0;
				else timer_mode = 1;
				while(Keypad() == 16);
			}
			if(a_mode == 1) {
				if(tmp>25) mode2 = 2;
				else mode2 = 1;
			}
		}
	}
}




//Timer Interrupt Control
void TIMER0_IRQHandler(void)
{
	uint8_t joystick_value = Joystick_read();
	if(Joystick_read() != 0) delay_time = 10;
	stepmotor_led_joystick(joystick_value);
	
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT); 
	
	if(a_mode == 1) Auto_Mode();
	if(timer_mode == 1){
		key =Keypad();
		if(key > 0)Time_Set();
	} 
	time_1s++;
	if(delay_time>0)delay_time--;
	if(time_1s == 60)
	{
		time_1m++;
		time_1s=0;
	
		if(time_1m == 10)
		{
			time_10m++;
			time_1m =0;
		}
		if(time_10m == 6)
		{
			time_1h++;
			time_10m = 0;
		}	
		if(time_1h == 10){
			time_10h++;
		}
		if(time_10h == 2 && time_1h == 4)
		{
			time_10h =0 ;
			time_1h =0 ;
			time_10m = 0;
			time_1m=0;
			time_1s = 0;
		}
	}
}



void TIMER0_Config(void)
{
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;	
	TIM_ConfigStruct.PrescaleValue	= 100;	
	TIM_MatchConfigStruct.MatchChannel = 0;	
	TIM_MatchConfigStruct.IntOnMatch = ENABLE;	
	TIM_MatchConfigStruct.ResetOnMatch = ENABLE;	
	TIM_MatchConfigStruct.StopOnMatch = DISABLE;	
	TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;	
	TIM_MatchConfigStruct.MatchValue = 10000;	
}
