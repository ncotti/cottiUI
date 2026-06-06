/***[Includes]****************************************************************/
#include "logger.h"
#include <stdio.h>
#include <string.h>

/***[Static variables]********************************************************/

/***[Static functions prototypes]*********************************************/

/***[Static functions]********************************************************/

/***[Public functions]********************************************************/

void logger_init(void) {
    FILE *log = fopen("log.txt", "w");
    fclose(log);
}

void logger_print(const char *str) {
    FILE *log = fopen("log.txt", "a");
    fwrite((void *)str, strlen(str), 1, log);
    fclose(log);
}
