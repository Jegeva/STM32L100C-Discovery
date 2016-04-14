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
uint8_t config_flags;


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
uint8_t BlinkSpeed = 0;

int threshold = 1000 * 10000;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */

//#define USE_FULL_ASSERT 1



//char message_ar[256];

/*
check ld script and startup, changed it so
only half of the mem is stack, half is heap
*/
/*heap starts here->*/
char * message_ar = (char*)0x20002000;


char * message;
int16_t last_temp;

volatile uint8_t message_available = 0;
volatile uint8_t temp_sampling_timer_fired;


int id_command(char* buff)
{
    int i,j;
    int cmd_ided = -1;
    for(i=0;i<commands_count;i++){
	j=0;
	while(*(commands[i]+j)==*(buff+j) )
	    j++;
	if(j){    
	    if(*(commands[i]+j)==0 || *(commands[i]+j-1)==0 ){
		cmd_ided=i;
		i=0xff;	    
	    }
	}
    }
    return cmd_ided;	
}

void help_for_command(char * buff)
{
    char help_mess_ar[64];
    int help_for_id;
    int i = 0;
    while(* (buff+i)!=0){ // whoopsie :pp
	*(help_mess_ar+i)=* (buff+i);
	i++;
    }
    help_for_id=id_command(buff+5);
    if(help_for_id==-1){
	usart_send_string("Unknown help topic");
	usart_send_string(commands_help[0]);
    } else {
	usart_send_string(commands_help[help_for_id]);
    }
    usart_CRLF();
   
    
}

void fCom_last_temp(char *buff){
     usart_send_MAX31820_temp(last_temp);
 usart_CRLF();  
};
void fCom_status(char *buff){
    	usart_send_string("Running");
	usart_CRLF();    
};
void fCom_reset_measures(char *buff){
    int offset = 1; // skip config flags and strings
    int len,i;    
    //int sample_nbr_offset;
    unsigned int sample_nbr=0;
    for(i=0;i<3;i++)
    {
	len = eeprom_read_byte_addr(offset);
	offset++;
	offset+=len;
    }
    offset++;
    //   sample_nbr_offset = offset;
    eeprom_write_bytearray_addr(offset,(uint8_t *)&sample_nbr,sizeof(unsigned int));

};
void fCom_count_measures(char *buff){
    int offset = 1; // skip config flags and strings
    int len,i;    
    //int sample_nbr_offset;
    unsigned int sample_nbr;
    for(i=0;i<3;i++)
    {
	len = eeprom_read_byte_addr(offset);
	offset++;
	offset+=len;
    }
    offset++;
    //   sample_nbr_offset = offset;
    eeprom_read_bytearray_addr(offset,(uint8_t *)&sample_nbr,sizeof(unsigned int));
    usart_send_uint(sample_nbr);
    
    usart_CRLF();    
};

void fCom_DisplaySamples(char* buff)
{   
    int i,len;
    unsigned int nbr,offset,sample_nbr;
    uint16_t t;
    
    nbr=0;
    offset=0;
    i=0;
    len=0;
    
    while(*(buff+len++)!=0); /*could there be a problem with the parsing code ?*/
    len--;
    while( (*(buff+i) < '0' || *(buff+i) > '9') && i<len  ){
	i++;
    }
    while( (*(buff+i) >= '0' && *(buff+i) <= '9') && i<len ){
	nbr*=10;
	nbr+= (*(buff+i))-'0';
	i++;
    }
    offset=1;
    for(i=0;i<3;i++)
    {
	len = eeprom_read_byte_addr(offset);
	offset++;
	offset+=len;
    }
    offset++;
    eeprom_read_bytearray_addr(offset,(uint8_t *)&sample_nbr,sizeof(unsigned int));
    offset += sizeof(unsigned int);
    if(nbr>sample_nbr)
	nbr=sample_nbr;
    
    for(i=0;i<nbr;i++){
	eeprom_read_bytearray_addr(offset,(uint8_t *)&t,sizeof(uint16_t));
	offset += sizeof(uint16_t);
	usart_send_MAX31820_temp(t);
	usart_CRLF();    
    }
    
}


void fCom_version(char *buff){
    int len=0;
    int offset=1;
    offset += eeprom_read_byte_addr(offset)+1;
    len = eeprom_read_byte_addr(offset)+1;
    eeprom_read_bytearray_addr(offset,(uint8_t *)message,len);
    *(message+len)=0;
    usart_send_string(message);	  
    usart_CRLF();
};
void fCom_threshold(char *buff){
    // char tmp_buff[11];
    int len=0;
    int tmp_int=0;
    int tmp_frac=0;
    
    int i=0;
    
    while(*(buff+len++)!=0); /*could there be a problem with the parsing code ?*/
    len--;
    if(len > 9){
	while( (*(buff+i) < '0' || *(buff+i) > '9') && i<len  ){
	    i++;
	}
	/*parse the int part*/
	while((*(buff+i) >= '0' && *(buff+i) <= '9') ){
	    tmp_int*=10;
	    tmp_int+=*(buff+i) - '0';
	  
	    i++;
	}
	if(tmp_int<1000){ /*checking the int par for > 1000 threshold*/
	    usart_send_string("threshold to low, not setting it\r\n");
	    return;
	}
	if(tmp_int>1000000){ /*checking the int par for > 1000 threshold*/
	    usart_send_string("threshold to high, not setting it\r\n");
	    return;
	}
//	tmp*=1000;
	
	if(*(buff+i) == '.' ){ // there is a frac part
	    i++;
	    
	    while((*(buff+i) >= '0' && *(buff+i) <= '9') ){
		tmp_frac*=10;
		tmp_frac+=*(buff+i) - '0';
		i++;
	    }
	    /*avoiding floating point by having >10000 int part , <10000 float part*/
	while(tmp_frac<625)
	    tmp_frac*=10;
	if(!(tmp_frac%625)){	    
	    threshold=tmp_int*10000+tmp_frac;
	} else {
	    usart_send_string("sensor only support frac part in 0.5 0.25 0.125 0.0625 increments, threshold not set\r\n");
	}
	}else{
	    threshold=tmp_int*10000;
	    
	}
	
	
    } else {
	usart_send_uint(threshold/10000);
	usart_send_byte('.');
	tmp_frac = threshold%10000;
	if(tmp_frac)
	while(tmp_frac<1000){
	    usart_send_byte('0');
	    tmp_frac*=10;
	    
	}
	usart_send_uint(threshold%10000);
	usart_CRLF();
    }
    
    
    
    //threshold

};

// __attribute__((target("thumb")))
void fCom___B00MBayST1Ck__(char *buff){
    if(! (config_flags & __CONFIG_DEBUG_BIT))
	return;  
    GPIOB->ODR |= (1<<4);
};

/*ld end of sections -> heap*/
extern void *_end;

uint8_t comp_last_tempthreshold()
{
    int tmp,res_temp,frac=0;
    if(last_temp & 0x8000){ //0b1000000000000000
	// neg, stored as 2s complement
	res_temp = -1;
	tmp = ~last_temp;
	tmp++;
    } else {
	res_temp = 1;
	tmp = last_temp;
    }
    if(tmp&1)
	frac +=625;
    if(tmp&2)
	frac +=1250;
    if(tmp&4)
	frac +=2500;
    if(tmp&8)
	frac +=5000;
    tmp = (tmp>>4);
    res_temp *= tmp*10000 + frac;
    return (res_temp>threshold);
    
    
    
}

extern volatile uint8_t usart_password;

#ifdef DIFFICULTY_LVL_1

char __xoredpass[39]={0xc8,0xa9,0xfa,0xaa,0xdc,0x8c,0x0f,0x0e,0xb1,0xf1,0xe3,0xc3,0xc4,0x85,0x07,0xd7,0xa9,0xfd,0xbe,0xdb,0xbc,0xed,0xbf,0xcf,0x93,0x10,0x13,0xa2,0xd4,0xb4,0xc6,0x76,0x29,0x89,0xea,0x1a,0x4c,0xc8,0x0};




    
volatile int8_t ided = 0;

int __validate_password(char * pass)
{
    char c;
    unsigned int i=0;
    unsigned int len=0; 
    while(*(__xoredpass+len)!=0)
	len++;   
    do{
	c =  ((~(( ( *(pass+i)   & 0x0f) << 4) | (( *(pass+i) & 0xf0 )>> 4)))^i);
    } while(  c == *(__xoredpass+i) && *(__xoredpass+i++)!=0 );   

    if(len==i)
    {
	return 1;
    } else {
	return 0;
    }    
}

#else

volatile int8_t ided = 0;

char *__pass="plaintext15not4reallygoodwaytoSt0#e";

int __validate_password(char * pass)
{
 
    unsigned int i=0;
    unsigned int len=0; 
    while(*(__pass+len)!=0)
	len++;   
    while( *(pass+i)==*(__pass+i) && ( *(pass+i) !=0) ){
	i++;
    }
    

    if(len==i)
    {
	return 1;
    } else {
	return 0;
    }    
}

#endif

#define MAX_STORED_SAMPLES 32512
// 127*1024/4 -> leave 1k alone for future strings

void store_temp(int16_t last_temp)
{
    uint16_t offset = 1; // skip config flags and strings
    int len,i;    
    int sample_nbr_offset;
    unsigned int sample_nbr;
    for(i=0;i<3;i++)
    {
	len = eeprom_read_byte_addr(offset);
	offset++;
	offset+=len;
    }
    offset++;
    sample_nbr_offset = offset;
    //usart_send_uint(sample_nbr_offset);
    //usart_CRLF();    
    eeprom_read_bytearray_addr(offset,(uint8_t *)&sample_nbr,sizeof(unsigned int));
    //usart_send_uint(sample_nbr_offset);
    //usart_CRLF();    
    //usart_send_uint(sample_nbr);
    //usart_CRLF();    
    
    offset += sizeof(unsigned int);
    sample_nbr = (sample_nbr+1) % MAX_STORED_SAMPLES;
    offset += ((sample_nbr)*sizeof(uint16_t));
    if(offset>=250)
	STM_EVAL_LEDToggle(LED3);
    //usart_send_uint(offset);
    //usart_CRLF();     usart_CRLF();    
    eeprom_write_bytearray_addr(offset,(uint8_t *)&last_temp,sizeof(uint16_t));
    eeprom_write_bytearray_addr(sample_nbr_offset,(uint8_t *)&sample_nbr,sizeof(unsigned int));
    
}

int main(void)
{

    int command_id;
    //   message_ar=(char*)0x20000040;
    message = message_ar;
    
    /* STM_EVAL_LEDInit(LED4);
       STM_EVAL_LEDInit(LED3);*/
    // SysTick end of count event each 1ms 
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIOB->ODR &= ~(1<<4);
    
    usart_init();
    // Configure LED3 and LED4 on STM32L100C-Discovery 
    STM_EVAL_LEDInit(LED3);
    STM_EVAL_LEDOn(LED4);
    int i,offset,len;
    USART_Cmd(USART1,ENABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // we received a byte
    usart_clrscrn();
    usart_set_receive_buff(message);
    
/*  usart_send_string("test!");
    usart_CRLF();  
    usart_send_uint(1234567890);
    usart_CRLF();*/
  
    eeprom_init();
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    //TIM_Cmd(TIM5, ENABLE);
    TIM_TimeBaseInitTypeDef TIM5_TimeBaseInitStruct;
    NVIC_InitTypeDef TIM5_NVIC_InitStructure;
    
    TIM5_TimeBaseInitStruct.TIM_Prescaler = RCC_Clocks.PCLK2_Frequency/5; // .2 sec
    TIM5_TimeBaseInitStruct.TIM_Period = 5;
    TIM5_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM5_TimeBaseInitStruct.TIM_ClockDivision =TIM_CKD_DIV4;
    // ->every 2sec
    
    TIM5_NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    TIM5_NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    TIM5_NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    TIM5_NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    
    TIM_TimeBaseInit(TIM5, &TIM5_TimeBaseInitStruct);

    NVIC_Init(&TIM5_NVIC_InitStructure);
    
    TIM_ITConfig(TIM5, TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM5, ENABLE);
    
   
    
#ifdef __WRITE_EEPROM
    offset= 0;
    
    uint8_t config =0;
    
#ifdef __CONFIG_DEBUG
    config |= __CONFIG_DEBUG_BIT ;
#endif
    eeprom_write_byte_addr(offset,config);
    offset++;
    for(i=0;i<3;i++)
    {
	
	eeprom_write_byte_addr(offset,eeprom_strings_lens[i]);
	offset++;
	eeprom_write_bytearray_addr(offset,(uint8_t *)eeprom_strings[i],eeprom_strings_lens[i]);
	offset += eeprom_strings_lens[i];
    }
    offset++;
    eeprom_write_int_addr(offset,0);
    
#endif
    
    len = 0;
    offset = 0;
    config_flags =  eeprom_read_byte_addr(offset);
    offset++;
    for(i=0;i<2;i++)
    {
	len = eeprom_read_byte_addr(offset);
	offset++;
	eeprom_read_bytearray_addr(offset,(uint8_t *)message,len);
	offset+=len;
	*(message+len)=0;
	usart_send_string(message);	  
	usart_CRLF();
    }
    
    STM_EVAL_LEDOff(LED4); // use to trig Logic Analyser-pc8
    //  max3182_init();
    usart_CRLF();
    last_temp = max3182_getTemp();

    // usart_send_MAX31820_temp(last_temp);
    usart_CRLF();  
    STM_EVAL_LEDOn(LED4); // use to trig Logic Analyser-pc8
    
    usart_send_string("Password:");usart_password=1;
    temp_sampling_timer_fired=1;
    
    while(1)
    {
	

	
	if(message_available){
	    message_available=0;
	    if(!ided){
		if(__validate_password(message)){
		    ided=1;
		    i=2;
		    len = eeprom_read_byte_addr(offset);
		    offset++;
		    eeprom_read_bytearray_addr(offset,(uint8_t *)message,len);
		    offset+=len;
		    *(message+len)=0;
		    usart_send_string(message); usart_CRLF();
		    usart_send_MAX31820_temp(last_temp);
		    
		    usart_CRLF();
		} else {
		    ided = 0;
		    usart_password=1;
		    usart_send_string("Password:");
		}
		
		
	    
	    
	    } else {

	    /*uart command*/    
	    
	    command_id=id_command(message);
	    if(command_id == -1){
		usart_send_string("Unknown command:");
		usart_send_string(message);
		usart_send_string(" (help for available commands)");
		usart_CRLF();  
	    } else {
		(*commands_function[command_id])(message);
	    }
	    
	    *message=0;
	    usart_set_receive_buff(message);
	    
	    }
	}
	
	if(comp_last_tempthreshold()){
	    GPIOB->ODR |= (1<<4); // boom, nuclear fallout and all, bad for the environment...
	}
	
//	Delay(1000);
	
	//STM_EVAL_LEDToggle(LED3);

	if(temp_sampling_timer_fired){
	    temp_sampling_timer_fired=0;
	    last_temp = max3182_getTemp();
	    store_temp(last_temp);
	    TIM5->DIER |= TIM_DIER_UIE;
	 
	}


	
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
