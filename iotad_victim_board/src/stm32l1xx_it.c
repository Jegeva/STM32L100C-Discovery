/**
  ******************************************************************************
  * @file    stm32l1xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    29-July-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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
#include "stm32l1xx_it.h"
#include "main.h"

/** @addtogroup STM32L100C-Discovery_Demo
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  TimingDelay_Decrement();
}

/******************************************************************************/
/*                 STM32L1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l1xx_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 
extern volatile unsigned char uart_ready_to_send;

extern volatile char * uart_curr_send;
extern volatile unsigned int uart_sent_nbr;

extern char * uart_mess_to_receive;
extern volatile char * uart_curr_receive;
extern volatile unsigned int uart_received_nbr;

unsigned char pending_LF = 0;


void  USART1_IRQHandler(void)
{
    unsigned char c;
    
    if(USART_GetITStatus(USART1,USART_IT_TXE) != RESET){
	USART_ClearITPendingBit(USART1,USART_IT_TXE);
	if(pending_LF){
	    USART1->DR = '\n';
	    pending_LF=0;	    
	    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	    USART_ClearITPendingBit(USART1,USART_IT_TXE);
	    uart_ready_to_send=1;
	    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	    return;
	    
	    
	}
	
	if(uart_curr_send == NULL){    
	    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	}

	if(*uart_curr_send==0){
	    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	    
	    uart_ready_to_send=1;
	} else {
	    USART1->DR = *uart_curr_send;
	    uart_curr_send++; 
	}
	USART_ClearITPendingBit(USART1,USART_IT_TXE);	
    }
    if(USART_GetITStatus(USART1,USART_IT_RXNE)){
	// echo
	c = USART1->DR;
	USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	((volatile USART_TypeDef*)USART1)->DR=c;
	if(uart_curr_receive !=NULL){
	    *uart_curr_receive++ = c;   
	}
	if(c=='\r'){
	    uart_ready_to_send=0;
	    pending_LF=1;
	    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	}
    }
}

extern volatile char MAX31820_tim_blocking;
extern volatile char MAX31820_detecting_pulses;
extern volatile unsigned char MAX31820_detecting_pulses_max;
volatile uint16_t MAX31820_detected_pulse_len;
extern volatile uint16_t MAX31820_detected_pulses;
extern volatile uint16_t * MAX31820_detected_pulses_len;
extern volatile unsigned char * MAX31820_detected_pulses_polarity;
extern volatile unsigned char MAX31820_pulse_front;


void TIM10_IRQHandler(void)
{
    if(TIM10->SR & TIM_SR_UIF){
	TIM10->SR &= ~TIM_SR_UIF;
	if(MAX31820_tim_blocking){
	    MAX31820_tim_blocking=0;	    
	}
    }
}


void EXTI9_5_IRQHandler(void)
{
    uint16_t tmp;
    // GPIOC->ODR ^= (1 << 8);
    
	 if( EXTI_GetITStatus(EXTI_Line5)!= RESET){
	     EXTI_ClearITPendingBit(EXTI_Line5); // DONT MOVE THIS
	      if(MAX31820_detecting_pulses){
	     tmp=TIM10->CNT;
	    
	     if(!MAX31820_pulse_front){
		 TIM10->CNT=0;
		 MAX31820_pulse_front=1;
		 MAX31820_detected_pulse_len=0;
		
	     } else {
	
		 MAX31820_pulse_front=0;	 
		 MAX31820_detected_pulse_len=tmp;		 
		 MAX31820_detected_pulses_polarity[MAX31820_detected_pulses]= EXTI->FTSR | EXTI_Line5 ? 1:0;
		 MAX31820_detected_pulses_len[MAX31820_detected_pulses++]=MAX31820_detected_pulse_len;
		 if(MAX31820_detected_pulses==MAX31820_detecting_pulses_max)
		     MAX31820_detecting_pulses=0;	 
	     }
	       if(EXTI->RTSR & EXTI_Line5){ // switch trigger polarity
		 EXTI->RTSR &= ~EXTI_Line5;
		 EXTI->FTSR |= EXTI_Line5;
	     } else {
		 EXTI->FTSR &= ~EXTI_Line5;
		 EXTI->RTSR |= EXTI_Line5;
		 }

	 }
     }
}
    



void I2C1_EV_IRQHandler(void)
{
    
}
void I2C1_ER_IRQHandler(void)
{
    
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
