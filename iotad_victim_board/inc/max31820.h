#ifndef __MAX31820_H
#define __MAX31820_H

#include "main.h"

#define PORT_MAX31820 GPIOB
#define PIN_MAX31820 GPIO_Pin_5
#define PIN_MAX31820_source  GPIO_PinSource5
#define TIM_MAX31820 TIM10
#define EXTI_PortSourceGPIOMAX31820 EXTI_PortSourceGPIOB
#define EXTI_PinSourceGPIOMAX31820 EXTI_PinSource5

void MAX31820_tim_block();

void MAX31820_write(uint8_t);

uint8_t MAX31820_read();

uint8_t MAX31820_block_readslot0();

int MAX31820_reset();

int16_t max3182_getTemp();


#endif
