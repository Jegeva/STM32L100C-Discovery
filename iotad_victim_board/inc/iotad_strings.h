#ifndef __IOTAD_STRINGS_H
#define __IOTAD_STRINGS_H

#include "main.h"

extern char * eeprom_strings [];
extern char eeprom_strings_lens[];
extern int commands_count;
extern char * commands[];
extern char * commands_help[];    
extern char * commands_help[];    
extern void (*commands_function[])(char*);

#endif
