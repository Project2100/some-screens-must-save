#ifndef SSMS_DEBUG_H
#define SSMS_DEBUG_H

#include <stdio.h>

// Logging stuff: The file is for general purpose, the info queue is used to dump d3d messages to another file
#define LOG_MAIN_FILENAME "application.log"
#define LOG_D3D_FILENAME "d3d.log"
#define LOG_CONFIG_FILENAME "config.log"

extern FILE* instanceLog;

extern FILE* devLog;

extern FILE* configLog;

#endif // SSMS_DEBUG_H
