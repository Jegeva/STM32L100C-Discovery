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






int main(void)
{




 STM_EVAL_LEDInit(LED4);
 STM_EVAL_LEDInit(LED3);
 // SysTick end of count event each 1ms 
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);


  usart_init();
  


// GPIO_InitTypeDef GPIO_InitStructure;


  
  // Configure LED3 and LED4 on STM32L100C-Discovery 
  STM_EVAL_LEDInit(LED3);
 
  
  //i2c
  STM_EVAL_LEDOn(LED4);
  
//  I2C_InitTypeDef I2C_InitStruct;

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
  
   eeprom_init();
  usart_send_byte(eeprom_read_byte_addr(0));
  usart_send_byte(eeprom_read_byte_addr(1));
  usart_send_byte(eeprom_read_byte_addr(2));

  // eeprom_read_int_addr(0);
 
  
  eeprom_write_int_addr(250,0xdeadbaaf);
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
