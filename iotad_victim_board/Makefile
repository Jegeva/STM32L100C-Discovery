SRCS = max31820.c 24lc.c usart.c main.c stm32l1xx_it.c system_stm32l1xx.c iotad_strings.c
REQS = max31820.h 24lc.h usart.h main.h stm32l1xx_it.h iotad_strings.h
PROJ_NAME=main

# that's it, no need to change anything below this line!

###################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size
GDB=gdb-multiarch

CFLAGS  = -g -O2 -Wall -Tstm32_flash.ld 
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3 -mthumb-interwork
CFLAGS += -msoft-float
FLASH_SIZE = 262144

###################################################

B := main

vpath %.c src
vpath %.h inc
vpath %.a lib

ROOT=$(shell pwd)

CFLAGS += -I../STM32L100C-Discovery_lib/inc -I../../CMSIS/Include/ -Iinc 
CFLAGS += -DARM_MATH_CM3 -lc -specs=nosys.specs
CFLAGS +=  -fdata-sections -ffunction-sections -Wl,--gc-sections -Os 

ifdef DEBUG
CFLAGS += -D__CONFIG_DEBUG -D__WRITE_EEPROM
endif

SRCS += startup_stm32l1xx_hd.s # add startup file to build

OBJS = $(SRCS:.c=.o) 

###################################################

.PHONY: lib proj

all: lib proj

lib:
	$(MAKE) -C ../STM32L100C-Discovery_lib/

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS) $(REQS)
	$(CC) $(CFLAGS) $^ -o $@ -Llib -L../STM32L100C-Discovery_lib -L../../CMSIS/Lib/GCC/ -larm_cortexM3l_math -lstm32l1 -lm
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	$(SIZE) $(PROJ_NAME).elf
	@perl -e 'printf "USED : %0.2f%% of '$(FLASH_SIZE)' bytes\n",100*('$(FLASH_SIZE)'/(100*'`ls -l main.bin |cut -d' ' -f5`')) '

prog: 
	st-flash write $(PROJ_NAME).bin 0x8000000

gdb :
	openocd --file /usr/share/openocd/scripts/board/stm32ldiscovery.cfg &
	sleep 1
	echo "target extended localhost:3333\nmonitor reset halt\nmonitor reset_nag enable\nb "$(B)"\nc\n" >.gdbinit
	$(GDB) main.elf && killall openocd

clean:
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f inc/*~ src/*~ inc/\#* src/\#* ./*~ 

allclean: clean
	$(MAKE) -C ../STM32L100C-Discovery_lib/ clean	
