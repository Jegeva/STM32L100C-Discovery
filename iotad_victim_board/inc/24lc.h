#ifndef __24LC_H
#define __24LC_H
#include "main.h"

#define PIN_I2C1_SDA GPIO_Pin_9
#define PIN_I2C1_SCL GPIO_Pin_8
#define PIN_I2C1_SDA_source  GPIO_PinSource9
#define PIN_I2C1_SCL_source  GPIO_PinSource8

#define addr_eeprom_7b 0x50
#define addr_eeprom_8b (addr_eeprom_7b<<1)

#define PORT_I2C1 GPIOB
void eeprom_init();

void eeprom_prelude();

uint8_t eeprom_read_byte_addr(uint16_t );

uint32_t eeprom_read_int_addr(uint16_t );

void eeprom_read_bytearray_addr(uint16_t,uint8_t* ,unsigned int);

void eeprom_write_byte_addr(uint16_t,uint8_t);
void eeprom_write_bytearray_addr(uint16_t ,uint8_t * ,unsigned int);

void eeprom_write_int_addr(uint16_t,uint32_t);

#endif
