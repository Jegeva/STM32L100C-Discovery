/**
  ******************************************************************************
  * @file    main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    29-July-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32L100C-Discovery_Demo
  * @{
  */



RCC_ClocksTypeDef RCC_Clocks;


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
uint8_t BlinkSpeed = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */

//#define USE_FULL_ASSERT 1

#define PIN_USART1_TX GPIO_Pin_6
#define PIN_USART1_RX GPIO_Pin_7
#define PIN_USART1_TX_Source GPIO_PinSource6
#define PIN_USART1_RX_Source GPIO_PinSource7

#define PIN_I2C1_SDA GPIO_Pin_9
#define PIN_I2C1_SCL GPIO_Pin_8
#define PIN_I2C1_SDA_source  GPIO_PinSource9
#define PIN_I2C1_SCL_source  GPIO_PinSource8

#define addr_eeprom_7b 0x50
#define addr_eeprom_8b (addr_eeprom_7b<<1)
#define PORT_USART1 GPIOB
#define PORT_I2C1 GPIOB


volatile unsigned char uart_ready_to_send=1;

char * uart_mess_to_send;
volatile char * uart_curr_send;
volatile unsigned int uart_sent_nbr;
char * uart_mess_to_receive;
volatile char * uart_curr_receive;
volatile unsigned int uart_received_nbr;




void usart_send_string(char * str)
{
    // wait until uart stopped sending the current thing
    while(uart_ready_to_send==0);
    uart_ready_to_send=0;
    
    uart_mess_to_send=str;
    uart_curr_send=str;
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);  // we sent a byte
    
//    USART_SendData(USART1,*str);
    
}
void usart_send_byte(char c)
{
    char str[2];
    str[0]=c;
    str[1]=0;
    usart_send_string(str);
     while(!uart_ready_to_send); // block until done , str is ON THE STACK
   
    
}

void usart_send_uint(uint32_t  n)
{
    // len(4294967295)=10
    int i;
    uint32_t tmp=n;
    char str[11];
    for(i=0;i<10;i++)
	str[i]=0;
    i=0;
    while(tmp){ // get nbr of digits
	tmp/=10;
	i++;
    }
    tmp=n;
    str[i]=0;
    str[0]='0';
    if(i)
	i--; // 0 based array
    
    while(i){
	str[i]='0'+tmp%10;
	tmp/=10;
	i--;
    }
    str[i]='0'+tmp;
    str[10]=0;
    usart_send_string(str);
    while(!uart_ready_to_send); // block until done , str is ON THE STACK 
}

void usart_send_MAX31820_temp(uint16_t t)
{
    uint16_t frac=0;
    char frac_str[5];
    uint16_t tmp;
    
    frac_str[4]=0;
   
    if(t & 0x8000){ //0b1000000000000000
	 usart_send_byte('-');
	  tmp=~t;
	  tmp++;
    }else{
	 tmp=t;
    }
    
     // no hard float ? float is for dumdums anyways...
     if(tmp&1)
	 frac +=625;
     if(tmp&2)
	 frac +=1250;
     if(tmp&4)
	 frac +=2500;
     if(tmp&8)
	 frac +=5000;
     tmp = (tmp>>4); // this ain't my dad, it's the float part ! throw it on the ground...
     usart_send_uint(tmp);
     usart_send_byte('.');
     frac_str[3]='0'+(frac%10);
     frac /=10;
     frac_str[2]='0'+(frac%10);
     frac /=10;
     frac_str[1]='0'+(frac%10);
     frac /=10;
     frac_str[0]='0'+(frac%10);
     frac_str[4]=0;
     usart_send_string(frac_str);
     while(!uart_ready_to_send); 
     
     
     
     
     
     
}



void usart_CRLF(void)
{
    usart_send_string("\r\n");
    
}

void usart_clrscrn()
{
    usart_send_string("\033[2J\033[H");
    
}

volatile int i2c_state;


void eeprom_prelude()
{
     I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1,addr_eeprom_8b ,I2C_Direction_Transmitter);  
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)){ // if 24lc writing -> NACKS
	if(I2C1->SR1 & (1<<10) ){ // Ack Fail
	    I2C1->SR1 = I2C1->SR1 & ~(1<<10); // clear flag
	    I2C_GenerateSTART(I2C1,ENABLE);
	    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	    I2C_Send7bitAddress(I2C1,addr_eeprom_8b ,I2C_Direction_Transmitter);


	}		
    }
}


uint8_t eeprom_read_byte_addr(uint16_t addr)
{
    uint8_t retval;
    
    eeprom_prelude();
    I2C_SendData(I2C1,  (uint8_t)addr>>8);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, (uint8_t)addr&0xff);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    //  I2C_GenerateSTOP(I2C1,ENABLE);
    I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));   
    I2C_Send7bitAddress(I2C1,addr_eeprom_8b ,I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    retval= I2C_ReceiveData(I2C1);
    // while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    I2C_GenerateSTOP(I2C1,ENABLE);
    
    return retval;
     
}

uint32_t eeprom_read_int_addr(uint16_t addr)
{
    uint32_t retval=0;
    char i=3;
    
    eeprom_prelude();
    I2C_SendData(I2C1,  (uint8_t)addr>>8);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, (uint8_t)addr&0xff);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    //  I2C_GenerateSTOP(I2C1,ENABLE);

    //seq read
    
    I2C_AcknowledgeConfig(I2C1, ENABLE); // ack 3 bytes
    I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));   
    I2C_Send7bitAddress(I2C1,addr_eeprom_8b ,I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    while(i!=0){
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	retval<<=8;
	retval |= I2C_ReceiveData(I2C1);
	i--;
	
    }
    
    I2C_AcknowledgeConfig(I2C1, DISABLE); // nack 1 byte
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    retval<<=8;
    retval |= I2C_ReceiveData(I2C1);
    
    I2C_GenerateSTOP(I2C1,ENABLE);
    
    return retval;
     
}

#define EEPROM_PAGE_SZ 64

// read stuff
// caveat emptor, alloc correct sz for array
// your vulns are not mine :p
void eeprom_read_bytearray_addr(uint16_t addr,uint8_t* array,unsigned int size)
{

    unsigned int full_pages_to_read = size/EEPROM_PAGE_SZ;
    unsigned int remaining_bytes_to_read = size%EEPROM_PAGE_SZ;
    unsigned int i,j;
    uint16_t r_addr = addr;
   
    for(i=0;i<full_pages_to_read;i++){    
	eeprom_prelude();
	I2C_SendData(I2C1,  (uint8_t)r_addr>>8);
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
	I2C_SendData(I2C1, (uint8_t)r_addr&0xff);
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
	//seq read
	I2C_AcknowledgeConfig(I2C1, ENABLE); // ack bytes but the last
	I2C_GenerateSTART(I2C1,ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));   
	I2C_Send7bitAddress(I2C1,addr_eeprom_8b ,I2C_Direction_Receiver);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	for(j=0;j<EEPROM_PAGE_SZ-1;j++){
	    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	    *(array+i*EEPROM_PAGE_SZ+j)= I2C_ReceiveData(I2C1);
	}
	I2C_AcknowledgeConfig(I2C1, DISABLE); // nack the last byte
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	*(array+i*EEPROM_PAGE_SZ+j)= I2C_ReceiveData(I2C1);
	I2C_GenerateSTOP(I2C1,ENABLE);
	r_addr+=EEPROM_PAGE_SZ;
    }
    if(remaining_bytes_to_read){
	eeprom_prelude();
	I2C_SendData(I2C1,  (uint8_t)r_addr>>8);
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
	I2C_SendData(I2C1, (uint8_t)r_addr&0xff);
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
	I2C_AcknowledgeConfig(I2C1, ENABLE); // ack bytes but the last
	I2C_GenerateSTART(I2C1,ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));   
	I2C_Send7bitAddress(I2C1,addr_eeprom_8b ,I2C_Direction_Receiver);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	for(i=0;i<remaining_bytes_to_read-1;i++){
	    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	    *(array+full_pages_to_read*EEPROM_PAGE_SZ+i)= I2C_ReceiveData(I2C1);
	}
	I2C_AcknowledgeConfig(I2C1, DISABLE); // nack the last byte
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	*(array+full_pages_to_read*EEPROM_PAGE_SZ+i)= I2C_ReceiveData(I2C1);
    }
    
}



void eeprom_write_byte_addr(uint16_t addr,uint8_t data)
{
    eeprom_prelude();
    I2C_SendData(I2C1,  (uint8_t)addr>>8);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, (uint8_t)addr&0xff);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, data);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_GenerateSTOP(I2C1,ENABLE);
}

void eeprom_write_int_addr(uint16_t addr,uint32_t data)
{
    eeprom_prelude();
    I2C_SendData(I2C1,  (uint8_t)addr>>8);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, (uint8_t)addr&0xff);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, (uint8_t)(data>>24));
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, (uint8_t)((data>>16)&0xff));
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, (uint8_t)((data>>8 )&0xff));
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, (uint8_t)((data    )&0xff));
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_GenerateSTOP(I2C1,ENABLE);
}

#define PORT_MAX31820 GPIOB
#define PIN_MAX31820 GPIO_Pin_5
#define PIN_MAX31820_source  GPIO_PinSource5
#define TIM_MAX31820 TIM10
#define EXTI_PortSourceGPIOMAX31820 EXTI_PortSourceGPIOB
#define EXTI_PinSourceGPIOMAX31820 EXTI_PinSource5
GPIO_InitTypeDef GPIO_MA31820X_InitStructure; // global because max uses tristate switch across funcs
TIM_TimeBaseInitTypeDef TIM_MAX31820_TimeBaseInitStruct;

volatile char MAX31820_tim_blocking;
volatile uint16_t MAX31820_detected_pulse_len;
volatile char MAX31820_detecting_pulses;
volatile unsigned char MAX31820_detecting_pulses_max;
volatile uint16_t * MAX31820_detected_pulses_len;
volatile unsigned char * MAX31820_detected_pulses_polarity;
volatile uint16_t MAX31820_detected_pulses;
volatile unsigned char MAX31820_pulse_front;

#define MAX31820_ROM_ReadRom 0x33
#define MAX31820_ROM_MatchRom 0x55
#define MAX31820_FUN_ConvertT 0x44
#define MAX31820_FUN_ReadSPad 0xbe
NVIC_InitTypeDef MAX31820_NVIC_InitStructure;


unsigned char crc_table_1w[256] = {
	0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
	157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
	35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
	190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
	70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
	219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
	101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
	248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
	140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
	17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
	175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
	50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
	202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
	87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
	233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
	116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53};

void MAX31820_tim_block()
{
   
        
   
    TIM_MAX31820->CNT=0;
    //TIM_MAX31820->EGR |=1;
    
    //   TIM_ITConfig(TIM_MAX31820,TIM_IT_Update,ENABLE); // hal is slow and this is time critical
    TIM10->DIER |= TIM_DIER_UIE;
    //TIM_Cmd(TIM10, ENABLE);
    // TIM10->CR1 = TIM_CR1_CEN;
    
    NVIC_Init(&MAX31820_NVIC_InitStructure);
       //NVIC_EnableIRQ(TIM10_IRQn);
    MAX31820_tim_blocking=1; // put it riiiight before blocking, enabling the nvic will fire spurious interrupts, intH will ignore if =0
    
    while(MAX31820_tim_blocking);
    // TIM_ITConfig(TIM10,TIM_IT_Update,DISABLE);
    TIM10->DIER &= ~TIM_DIER_UIE;
   
}

void MAX31820_write(uint8_t command)
{
    uint8_t tmp = command;
    char i = 8;
    
    GPIO_MA31820X_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
    GPIOB->ODR |= (1 << 5); // PUT HIGH BEFORE SET OR ELSE THE SETUP TIME WILL BOTCH THE TIMINGS !
    GPIO_Init(PORT_MAX31820, &GPIO_MA31820X_InitStructure);
    TIM_ITConfig(TIM_MAX31820,TIM_IT_Update,ENABLE);
    TIM_UpdateRequestConfig(TIM_MAX31820,TIM_UpdateSource_Regular);


 
	
    while(i--){
	
	TIM_MAX31820_TimeBaseInitStruct.TIM_Period = 1;
	TIM_TimeBaseInit(TIM_MAX31820, &TIM_MAX31820_TimeBaseInitStruct);
	GPIO_ResetBits(PORT_MAX31820,PIN_MAX31820);
	TIM10->CR1 = TIM_CR1_CEN;
	MAX31820_tim_block();

//	if(tmp&(1<<7)){
	if(tmp&1)
	    {		
		//GPIO_SetBits(PORT_MAX31820,PIN_MAX31820);
		GPIOB->ODR |= (1 << 5);
	    }
	TIM_MAX31820_TimeBaseInitStruct.TIM_Period = 70;
	TIM_TimeBaseInit(TIM_MAX31820, &TIM_MAX31820_TimeBaseInitStruct);
	MAX31820_tim_block();
	GPIO_SetBits(PORT_MAX31820,PIN_MAX31820);
	TIM_MAX31820_TimeBaseInitStruct.TIM_Period = 5;
	TIM_TimeBaseInit(TIM_MAX31820, &TIM_MAX31820_TimeBaseInitStruct);
	MAX31820_tim_block();
	//tmp<<=1;
	tmp>>=1;
	
    }
    TIM_Cmd(TIM10, DISABLE);
    GPIO_MA31820X_InitStructure.GPIO_Mode =  GPIO_Mode_IN; // for later
    GPIO_Init(PORT_MAX31820, &GPIO_MA31820X_InitStructure);    
}

uint8_t MAX31820_read()
{
    uint8_t tmp = 0;
    char i = 8;
    
    TIM_ITConfig(TIM_MAX31820,TIM_IT_Update,ENABLE);
    TIM_UpdateRequestConfig(TIM_MAX31820,TIM_UpdateSource_Regular);
    while(i--){
	tmp>>=1;
	GPIO_MA31820X_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
	GPIOB->ODR |= (1 << 5); // PUT HIGH BEFORE SET OR ELSE THE SETUP TIME WILL BOTCH THE TIMINGS !
	GPIO_Init(PORT_MAX31820, &GPIO_MA31820X_InitStructure);
	TIM_MAX31820_TimeBaseInitStruct.TIM_Period = 1;
	TIM_TimeBaseInit(TIM_MAX31820, &TIM_MAX31820_TimeBaseInitStruct);
	GPIO_ResetBits(PORT_MAX31820,PIN_MAX31820);
	TIM10->CR1 = TIM_CR1_CEN;
	GPIO_MA31820X_InitStructure.GPIO_Mode =  GPIO_Mode_IN; //for later
	MAX31820_tim_block();
	GPIO_Init(PORT_MAX31820, &GPIO_MA31820X_InitStructure);
	GPIOC->ODR ^= (1 << 8); // see samples on log analyser
	tmp|=(GPIOB->IDR & (1 << 5))<<2;
	GPIOC->ODR ^= (1 << 8); // see samples on log analyser
	TIM_MAX31820_TimeBaseInitStruct.TIM_Period = 35;
	TIM_TimeBaseInit(TIM_MAX31820, &TIM_MAX31820_TimeBaseInitStruct);
	MAX31820_tim_block();

	
    }
    return tmp;
}

uint8_t MAX31820_block_readslot0()
{
    uint8_t tmp = 0;
  
    
    TIM_ITConfig(TIM_MAX31820,TIM_IT_Update,ENABLE);
    TIM_UpdateRequestConfig(TIM_MAX31820,TIM_UpdateSource_Regular);
    while(!tmp){
	GPIO_MA31820X_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
	GPIOB->ODR |= (1 << 5); // PUT HIGH BEFORE SET OR ELSE THE SETUP TIME WILL BOTCH THE TIMINGS !
	GPIO_Init(PORT_MAX31820, &GPIO_MA31820X_InitStructure);
	TIM_MAX31820_TimeBaseInitStruct.TIM_Period = 1;
	TIM_TimeBaseInit(TIM_MAX31820, &TIM_MAX31820_TimeBaseInitStruct);
	GPIO_ResetBits(PORT_MAX31820,PIN_MAX31820);
	TIM10->CR1 = TIM_CR1_CEN;
	GPIO_MA31820X_InitStructure.GPIO_Mode =  GPIO_Mode_IN; //for later
	MAX31820_tim_block();
	GPIO_Init(PORT_MAX31820, &GPIO_MA31820X_InitStructure);
	GPIOC->ODR ^= (1 << 8); // see samples on log analyser
	tmp|=(GPIOB->IDR & (1 << 5))<<2;
	GPIOC->ODR ^= (1 << 8); // see samples on log analyser
	TIM_MAX31820_TimeBaseInitStruct.TIM_Period = 35;
	TIM_TimeBaseInit(TIM_MAX31820, &TIM_MAX31820_TimeBaseInitStruct);
	MAX31820_tim_block();
//	tmp>>=1;
	
    }
    return tmp;
}



int MAX31820_reset()
{
    
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;
    unsigned char pols[1];
    uint16_t lens[1];
   
    
    MAX31820_detected_pulses_polarity=pols;
    MAX31820_detected_pulses_len = lens;
    MAX31820_detecting_pulses_max=1;
    MAX31820_detected_pulses=0;
    
    // en clock for TIM 10 is gen purpose, leave more specialised avail. no prescaler = 32MHz
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10,ENABLE);

    // we want to count usecs -> prescaler = 32000-1
    // we will use it in 2 ways :
    // 1) make a blocking function that returns on the period interupt a bit like the default delay (bottom of source code)
    // 2) have it free running to measure time on exti (interrupt on gpio change, more on that later)

    // set it up for mode 1) for the 480us init pulse of the max (cf. max DS p16)
    TIM_MAX31820_TimeBaseInitStruct.TIM_Prescaler = RCC_Clocks.PCLK2_Frequency/1000000;
    TIM_MAX31820_TimeBaseInitStruct.TIM_Period = 480;
    TIM_MAX31820_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_MAX31820_TimeBaseInitStruct.TIM_ClockDivision =TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM_MAX31820, &TIM_MAX31820_TimeBaseInitStruct);

    //setup gpio ATM out for the pull down of the init pulse
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);   
    GPIO_MA31820X_InitStructure.GPIO_Pin = PIN_MAX31820 ;
    GPIO_MA31820X_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
    GPIO_MA31820X_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_MA31820X_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_MA31820X_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(PORT_MAX31820, &GPIO_MA31820X_InitStructure);

    //setup NVIC (periph handling interrupts)
    MAX31820_NVIC_InitStructure.NVIC_IRQChannel = TIM10_IRQn;
    MAX31820_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    MAX31820_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    MAX31820_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // NVIC_Init(&MAX31820_NVIC_InitStructure);
    TIM_UpdateRequestConfig(TIM_MAX31820,TIM_UpdateSource_Regular);
    
    GPIO_ResetBits(PORT_MAX31820,PIN_MAX31820);
    GPIO_MA31820X_InitStructure.GPIO_Mode =  GPIO_Mode_IN; // for later
    //  NVIC_Init(&MAX31820_NVIC_InitStructure);
    	TIM10->CR1 = TIM_CR1_CEN;
    MAX31820_tim_block();  
    GPIO_Init(PORT_MAX31820, &GPIO_MA31820X_InitStructure);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOMAX31820,PIN_MAX31820_source);
    EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_Line=EXTI_Line5;
    EXTI_InitStruct.EXTI_LineCmd=ENABLE;
    EXTI_Init(&EXTI_InitStruct);
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  
   
    MAX31820_detected_pulses=0;
    MAX31820_pulse_front=0;
    
    
    NVIC_Init(&NVIC_InitStructure);
    TIM_MAX31820_TimeBaseInitStruct.TIM_Period = 0xffff;
    TIM_TimeBaseInit(TIM_MAX31820, &TIM_MAX31820_TimeBaseInitStruct);
    TIM_Cmd(TIM10, ENABLE);
   
    
    MAX31820_detecting_pulses=1;  
 
    while(MAX31820_detecting_pulses);

    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    EXTI_DeInit();
    SYSCFG->EXTICR[PIN_MAX31820_source >> 0x02]=0;//disable exti
    
    /* usart_send_byte(pols[0]?'0':'1');
    usart_send_byte(':');
    usart_send_uint(lens[0]);*/
    
    TIM_Cmd(TIM10, DISABLE);

    if(MAX31820_detected_pulses==0 || lens[0]>240 || lens[0]<60 ){
	//error, no presence pulse
	return -1;
	
    } else {
	return 1;
	
    }
    
}

unsigned char check_crc_uid(uint8_t *uid)
{
    unsigned char crc=0;
     unsigned char i=0;
     for(i=0;i<7;i++)
	 crc =  crc_table_1w[crc^uid[i]];
     return (crc==uid[7]);
     
     
}
unsigned char check_crc_spad(uint8_t *uid)
{
    unsigned char crc=0;
     unsigned char i=0;
     for(i=0;i<8;i++)
	 crc =  crc_table_1w[crc^uid[i]];
     return (crc==uid[8]);
     
     
}

int16_t max3182_getTemp()
{
    uint8_t uid[8];
    uint8_t spad[9];
    uint8_t spad_crc_ok=0;
    
    int i;
    
    MAX31820_reset();
    MAX31820_write(MAX31820_ROM_ReadRom);
    // usart_CRLF();
    for(i=0;i<8;i++)
	uid[i]=MAX31820_read();

    if(check_crc_uid(uid)){
	GPIOC->ODR ^= (1 << 9);
	GPIOC->ODR ^= (1 << 9);
    }
    
    
    MAX31820_write(MAX31820_FUN_ConvertT);
    MAX31820_block_readslot0();
    while(!spad_crc_ok){
	
    MAX31820_reset();
    MAX31820_write(MAX31820_ROM_MatchRom);
  
    GPIOC->ODR ^= (1 << 8); // see samples on log analyser

    for(i=0;i<8;i++)
	MAX31820_write(uid[i]);

    GPIOC->ODR ^= (1 << 8); // see samples on log analyser
    
    //while(!MAX31820_read);
    MAX31820_write(MAX31820_FUN_ReadSPad);
    for(i=0;i<9;i++)
	spad[i]=MAX31820_read();

    spad_crc_ok=check_crc_spad(spad);
    

    }
    
    // cf. max DS p 6

    return ((int16_t)spad[0]|(((int16_t)spad[1])<<8));
    
    
}


int main(void)
{


  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  //configure uart
  USART_ClockInitTypeDef USART_ClockInitStruct;
  USART_InitTypeDef USART_InitStruct;

 STM_EVAL_LEDInit(LED4);
 STM_EVAL_LEDInit(LED3);
 // SysTick end of count event each 1ms 
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  //UART settings, baudrate,...
  USART_InitStruct.USART_BaudRate=9600;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits= USART_StopBits_1;  
  USART_InitStruct.USART_Parity=USART_Parity_No;
  USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
  // UART clock
  USART_ClockInitStruct.USART_Clock = USART_Clock_Disable;
  USART_ClockInitStruct.USART_CPOL = USART_CPOL_Low;
  USART_ClockInitStruct.USART_CPHA = USART_CPHA_1Edge;
  USART_ClockInitStruct.USART_LastBit = USART_LastBit_Disable;
  // Configure GPIO mode 
  GPIO_InitStructure.GPIO_Pin =  PIN_USART1_TX| PIN_USART1_RX;
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;


  
  // Enable the USARTx Interrupt 
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  //apply gpio conf and set AF
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);       
  GPIO_PinAFConfig(PORT_USART1,PIN_USART1_RX_Source,GPIO_AF_USART1 );
  GPIO_PinAFConfig(PORT_USART1,PIN_USART1_TX_Source,GPIO_AF_USART1 );
  GPIO_Init(PORT_USART1, &GPIO_InitStructure);
  USART_Init(USART1, &USART_InitStruct);
  USART_ClockInit(USART1, &USART_ClockInitStruct);

  NVIC_Init(&NVIC_InitStructure);
  
  // Configure LED3 and LED4 on STM32L100C-Discovery 
  STM_EVAL_LEDInit(LED3);
 
  
  //i2c
  STM_EVAL_LEDOn(LED4);
  
  I2C_InitTypeDef I2C_InitStruct;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  
  I2C_InitStruct.I2C_ClockSpeed = 5000;
  I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStruct.I2C_OwnAddress1 = 0;
  I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
  I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

  
  GPIO_InitStructure.GPIO_Pin =  PIN_I2C1_SDA|PIN_I2C1_SCL;
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);       

  GPIO_PinAFConfig(PORT_I2C1,PIN_I2C1_SDA_source,GPIO_AF_I2C1 );
  GPIO_PinAFConfig(PORT_I2C1,PIN_I2C1_SCL_source,GPIO_AF_I2C1 );
  GPIO_Init(PORT_I2C1, &GPIO_InitStructure);
  
 
  I2C_Init(I2C1,&I2C_InitStruct);
/*
  NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);
*/
  
  I2C_Cmd(I2C1,ENABLE);


 
/*  eeprom_write_byte_addr(0,'I');
  eeprom_write_byte_addr(1,'0');
  eeprom_write_byte_addr(2,'T');

*/
   
  // Initiate Blink Speed variable 
  BlinkSpeed = 1;

  USART_Cmd(USART1,ENABLE);
 
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // we received a byte
  usart_clrscrn();
  usart_send_string("test!");
  usart_CRLF();  
  usart_send_uint(1234567890);
  usart_CRLF();  
  
  usart_send_byte(eeprom_read_byte_addr(0));
  usart_send_byte(eeprom_read_byte_addr(1));
  usart_send_byte(eeprom_read_byte_addr(2));

  // eeprom_read_int_addr(0);
  
  eeprom_write_int_addr(250,0xdeadbeef);
  eeprom_read_int_addr(250);

/*  uint8_t test_ar[257];
  eeprom_read_bytearray_addr(0,test_ar,256);
  test_ar[256]=0;
  usart_send_string((char*)test_ar);
*/
   STM_EVAL_LEDOff(LED4); // use to trig Logic Analyser-pc8
   //  max3182_init();
   usart_CRLF();  
   usart_send_MAX31820_temp(max3182_getTemp());
   usart_CRLF();  
   STM_EVAL_LEDOn(LED4); // use to trig Logic Analyser-pc8
   
  while(1)
    {
	Delay(1000);
	
	STM_EVAL_LEDToggle(LED3);
    }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in 1 ms.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
volatile int ass_line;

void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
    ass_line = line;
    
  while (1)
  {}
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
