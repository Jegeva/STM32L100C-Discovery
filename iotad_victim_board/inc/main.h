/**
  ******************************************************************************
  * @file    main.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    29-July-2013
  * @brief   Header for main.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx.h"
#include "stm32l100c_discovery.h"
#include "stm32l100c_discovery.h"
#include "stm32l1xx_conf.h"
#include "stm32l1xx_usart.h"
#include "stm32l1xx_i2c.h"
#include "stm32l1xx_syscfg.h"
#include "stm32l1xx_exti.h"
#include "system_stm32l1xx.h"
#include "usart.h"
#include "24lc.h"
#include "max31820.h"
#include "iotad_strings.h"
#include <stdlib.h>
/* Exported types ------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define __CONFIG_DEBUG_BIT 0x80

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void TimingDelay_Decrement(void);
void Delay(__IO uint32_t nTime);
void usart_send_string(char * str);


void help_for_command(char *);
void fCom_last_temp(char *);
void fCom_status(char *);
void fCom_reset_measures(char *);
void fCom_count_measures(char *);
void fCom_version(char *);
void fCom_threshold(char *);
void fCom_DisplaySamples(char*);

void fCom___B00MBayST1Ck__(char *);
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
