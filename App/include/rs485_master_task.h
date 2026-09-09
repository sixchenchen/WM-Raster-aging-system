#ifndef __RS485_MASTER_TASK_H
#define __RS485_MASTER_TASK_H

#include "system_config.h"

#define RESPONSE_TIMEOUT_MS 80
#define DISCOVERY_TIMEOUT_FAST_MS 20 
#define RETRY_COUNT 3
#define DISCOVERY_INTERVAL_MS 20000  


/* master State */
typedef enum
{
    MASTER_SEND,
    MASTER_WAIT,
    MASTER_DISCOVERY_SEND,
    MASTER_DISCOVERY_WAIT,
} Master_State;

void RS485_Master_Task(void);

#endif
