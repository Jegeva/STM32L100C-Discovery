#ifndef __USART_H
#define __USART_H
#include "main.h"

#define PIN_USART1_TX GPIO_Pin_6
#define PIN_USART1_RX GPIO_Pin_7
#define PIN_USART1_TX_Source GPIO_PinSource6
#define PIN_USART1_RX_Source GPIO_PinSource7
#define PORT_USART1 GPIOB
void usart_init();


void usart_send_string(char *);

void usart_send_byte(char );

void usart_send_uint(uint32_t  );

void usart_send_MAX31820_temp(uint16_t );

void usart_CRLF(void);

void usart_clrscrn();

#endif
