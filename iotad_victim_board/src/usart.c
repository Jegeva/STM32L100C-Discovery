#include "usart.h"

volatile unsigned char uart_ready_to_send=1;

char * uart_mess_to_send;
volatile char * uart_curr_send;
volatile unsigned int uart_sent_nbr;
char * uart_mess_to_receive;
volatile char * uart_curr_receive;
volatile unsigned int uart_received_nbr;


void usart_set_receive_buff(char * buff)
{
    uart_curr_receive=uart_mess_to_receive=buff;
    
}


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

void usart_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
  
  //configure uart
    USART_ClockInitTypeDef USART_ClockInitStruct;
    USART_InitTypeDef USART_InitStruct;
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

}
