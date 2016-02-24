#include "24lc.h"

volatile int i2c_state;

void eeprom_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
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
    I2C_Cmd(I2C1,ENABLE);
}


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



void eeprom_write_bytearray_addr(uint16_t addr,uint8_t * data,unsigned int size)
{
    int sz = size;
    uint8_t * ptr = data;
    
    eeprom_prelude();
    I2C_SendData(I2C1,  (uint8_t)addr>>8);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    I2C_SendData(I2C1, (uint8_t)addr&0xff);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    while(sz--){	
    I2C_SendData(I2C1, *ptr++);
    while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) );
    }
    
    I2C_GenerateSTOP(I2C1,ENABLE);
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
