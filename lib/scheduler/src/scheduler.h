#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*scheduler_task_func)(void);

typedef unsigned long (*millis_func)(void);

#define SCHEDULER_STOP_TASK 0


/**
 * Init function to give function pointer to millis() function
 * 
 * @param millis_function_impl  Funtion pointer for getting time in milliseconds
 */
void scheduler_init(millis_func millis_function_impl);

/**
 * Add task to scheduler queue
 * 
 * @param func          Funtion pointer for task
 * @param call_after_ms Delay when task should be called first time. Zero to call it immediately.
 */
void scheduler_add_task(scheduler_task_func task, uint32_t call_after_ms);

/**
 * Loop function for scheduler
 *
 * @return time for next event in milliseconds
 */
uint32_t scheduler_loop(void);

/**
 * Clear all tasks from scheduler
 */
void scheduler_clear_all(void);

#ifdef __cplusplus
}
#endif

#endif // SCHEDULER_H
