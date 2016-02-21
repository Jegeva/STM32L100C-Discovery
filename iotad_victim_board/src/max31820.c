#include "max31820.h"


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
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
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
