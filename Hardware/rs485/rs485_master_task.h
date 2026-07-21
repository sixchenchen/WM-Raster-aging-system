#ifndef __RS485_MASTER_TASK_H
#define __RS485_MASTER_TASK_H

#define SLAVE_COUNT 16
#define RESPONSE_TIMEOUT_MS 20
#define RETRY_COUNT 3

/* master State */
typedef enum
{
    MASTER_SEND,
    MASTER_WAIT,
} Master_State;

void RS485_Master_Task(void);

#endif
