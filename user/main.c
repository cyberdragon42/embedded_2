#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "main.h"

#define 	LCD_PORT				 GPIOE
#define   LCD_PIN_RS       GPIO_Pin_7  
#define   LCD_PIN_RW       GPIO_Pin_10
#define   LCD_PIN_EN       GPIO_Pin_11          
#define   LCD_PIN_D7       GPIO_Pin_15         
#define   LCD_PIN_D6       GPIO_Pin_14          
#define   LCD_PIN_D5       GPIO_Pin_13         
#define   LCD_PIN_D4       GPIO_Pin_12         
#define   LCD_PIN_MASK    ((LCD_PIN_RS | LCD_PIN_EN | LCD_PIN_D7 | LCD_PIN_D6 | LCD_PIN_D5 | LCD_PIN_D4))
#define 	LCD_PIN_OFFSET  12

/*
char message1[] = {'h','e','l','l','o'};
char message2[] = {'w','o','r','l','d'};
char message3[] = {'g','p','i','o','e'};
char message4[] = {'e','m','b','e','d','d','e','d'};
char message5[] = {'s','t','m','3','2','f','4','v','g'};
*/

char *strs[5] = {"hello", "world", "gpioe","embedded","stm32f4vg"};
int str_index = 0;

int main(void)
{
	InitializePeripherals();
	InitializeDisplay();
	while(1)
	{
		
	}
}


//interrupt handler
void EXTI0_IRQHandler()
{
  //action after btn press
	ClearDisplay();
	WriteString(strs[str_index]);
	++str_index;
	if(str_index == 5)
		str_index = 0;
  EXTI_ClearITPendingBit(EXTI_Line0);
}


void InitializePeripherals()
{
	 SysTick_Config(SystemCoreClock/1000); 
	
	 //lcd outputs
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	 GPIO_InitTypeDef  GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = LCD_PIN_RS | LCD_PIN_RW | LCD_PIN_EN | LCD_PIN_D7 | LCD_PIN_D6 | LCD_PIN_D5 | LCD_PIN_D4;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOE, &GPIO_InitStructure);
   GPIO_ResetBits(LCD_PORT, LCD_PIN_RW);
	
	 //button
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	 GPIO_InitTypeDef PORTA;
	 GPIO_StructInit(&PORTA);
   PORTA.GPIO_Mode=GPIO_Mode_IN;
   PORTA.GPIO_Pin = GPIO_Pin_0;
   PORTA.GPIO_Speed = GPIO_Speed_2MHz;
   PORTA.GPIO_PuPd = GPIO_PuPd_DOWN;
   GPIO_Init(GPIOA,&PORTA);
	 
	 //external interruption
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
	 EXTI_InitTypeDef exti1;
	 EXTI_StructInit(&exti1);
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);
   exti1.EXTI_Line = EXTI_Line0;
   exti1.EXTI_LineCmd = ENABLE;
   exti1.EXTI_Mode = EXTI_Mode_Interrupt;
   exti1.EXTI_Trigger = EXTI_Trigger_Rising;
   EXTI_Init(&exti1);

   NVIC_EnableIRQ(EXTI0_IRQn);
   __enable_irq();
}
	
	
//write byte
void WriteByte(uint8_t data, int dataORcommand)
{
	if(dataORcommand == 0)//RS=0 (command) | 1 (data)
    GPIO_ResetBits(LCD_PORT, LCD_PIN_RS);
	else
		GPIO_SetBits(LCD_PORT, LCD_PIN_RS);
	
  GPIO_ResetBits(LCD_PORT, LCD_PIN_EN);// E=0
	
	// 0ldest halfbyte DB4…DB7
	 if(((data>>7)&1)==1) GPIO_SetBits(LCD_PORT, LCD_PIN_D7);
	else GPIO_ResetBits(LCD_PORT, LCD_PIN_D7);
	
		if(((data>>6)&1)==1) GPIO_SetBits(LCD_PORT, LCD_PIN_D6);
	else GPIO_ResetBits(LCD_PORT, LCD_PIN_D7);
	
		if(((data>>5)&1)==1) GPIO_SetBits(LCD_PORT, LCD_PIN_D5);
	else GPIO_ResetBits(LCD_PORT, LCD_PIN_D7);
	
		if(((data>>4)&1)==1) GPIO_SetBits(LCD_PORT, LCD_PIN_D4);
	else GPIO_ResetBits(LCD_PORT, LCD_PIN_D7);
	
  GPIO_SetBits(LCD_PORT, LCD_PIN_EN);// E=1
	lcd_delay(1000);
  GPIO_ResetBits(LCD_PORT, LCD_PIN_EN);// E=0
	GPIO_ResetBits(LCD_PORT, 0x0F<<LCD_PIN_OFFSET);
	lcd_delay(1000);
	
	// youngest halfbyte DB4…DB7
		if(((data>>3)&1)==1) GPIO_SetBits(LCD_PORT, LCD_PIN_D7);
	else GPIO_ResetBits(LCD_PORT, LCD_PIN_D7);
	
		if(((data>>2)&1)==1) GPIO_SetBits(LCD_PORT, LCD_PIN_D6);
	else GPIO_ResetBits(LCD_PORT, LCD_PIN_D7);
	
		if(((data>>1)&1)==1) GPIO_SetBits(LCD_PORT, LCD_PIN_D5);
	else GPIO_ResetBits(LCD_PORT, LCD_PIN_D7);
	
		if(((data>>0)&1)==1) GPIO_SetBits(LCD_PORT, LCD_PIN_D4);
	else GPIO_ResetBits(LCD_PORT, LCD_PIN_D7);
	
  GPIO_SetBits(LCD_PORT, LCD_PIN_EN);// E=1
	lcd_delay(1000);
  GPIO_ResetBits(LCD_PORT, LCD_PIN_EN);// E=0
	lcd_delay(1000);
}	

void SetCursore(uint8_t col, uint8_t row)
{
 	col |= 0x80;
	if (row == 1) {
     col += 0x40;
	}
 WriteByte(col, 0);
}

void WriteSymbol(char symbol)
{
   WriteByte(symbol,1);
}

void WriteCommand(char cmd)
{
   WriteByte(cmd,0);
}

void WriteString(char* str)
{
	  char *c;
    c = str;
    while ((c != 0) && (*c != 0))
    {
        WriteSymbol(*c);
        ++c;
    }
}

void ClearDisplay()
{
	//send command 0x01
	 WriteCommand(0x01);
	 lcd_delay(1000);
}

void InitializeDisplay()
{
	WriteCommand(0x20);
	lcd_delay(1000);
	WriteCommand(0x28);
	lcd_delay(1000);
	WriteCommand(0x28);
	lcd_delay(1000);
	WriteCommand(0x0F);
	lcd_delay(1000);
	WriteCommand(0x01);
	lcd_delay(1000);
	WriteCommand(0x06);
}

void lcd_delay(int p)
{
	int i;
	for(i=0;i<(p*10);i++){
	__nop();
	}
}

