#include "iotad_strings.h"

char * eeprom_strings []= {
    "IoTAD victim board",
    "v0.9b",
    "type 'help' for commands"
};


char eeprom_strings_lens[] = {
    18,
    5,
    24
};

int commands_count = 9;


char * commands[] = {
    "help",
    "status",
    "last_temp",
    "reset_measures",
    "count_measures",
    "version",
    "threshold",
    "display_samples",
    "__B00MBayST1Ck__"
};


char * commands_help[] = {
    "help, displays this help.\r\n"
    "available commands : help, status, last_temp, reset_measures, count_measures, version, threshold,display_samples",
    "status, displays the status of the device",
    "last_temp, displays the last temperature measured",
    "reset_measures, deletes all measures and starts over ",
    "count_measures, displays the number of measures available",
    "version, displays the firmware version",
    "threshold [New threshold], displays the temperature threshold, sensor only support frac part in 0.5 0.25 0.125 0.0625 increments",
    "display_samples [number] displays number samples taken since the start of sampling",
    ""
};

void (*commands_function[])(char*) = 
{
    help_for_command,
    fCom_status,
    fCom_last_temp,
    fCom_reset_measures,
    fCom_count_measures,
    fCom_version,
    fCom_threshold,
    fCom_DisplaySamples,
    fCom___B00MBayST1Ck__
};

    

    
    
    


