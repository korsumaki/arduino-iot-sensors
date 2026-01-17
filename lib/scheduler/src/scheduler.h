#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*scheduler_task_func)(void);

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
 * @param current_time_ms   Current system timestamp in milliseconds
 * @return time for next event in milliseconds
 */
uint32_t scheduler_loop(uint32_t current_time_ms);

/**
 * Clear all tasks from scheduler
 */
void scheduler_clear_all(void);

#ifdef __cplusplus
}
#endif

#endif // SCHEDULER_H
