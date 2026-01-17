#include "scheduler.h"
#include "stddef.h" // NULL
#include "assert.h"
//#include <stdio.h>

#define SCHEDULER_TASK_COUNT 5

typedef struct 
{
    uint32_t next_call_ms;
    scheduler_task_func task;
} scheduler_item_t;

static scheduler_item_t task_queue[SCHEDULER_TASK_COUNT] = { 0 };

static uint32_t scheduler_current_time_ms = 0;

void scheduler_clear_all(void)
{
    for (uint32_t index = 0; index < SCHEDULER_TASK_COUNT; index++)
    {
        task_queue[index].task = NULL;
        task_queue[index].next_call_ms = 0;
    }
}

void scheduler_print_table(void)
{
    //printf("Scheduler table:\n");
    for (uint32_t index = 0; index < SCHEDULER_TASK_COUNT; index++)
    {
        void* task = task_queue[index].task;
        if (task != NULL)
        {
            //printf("  #%d: %p, %d\n", index, task, task_queue[index].next_call_ms);
        }
    }
}

static void scheduler_sort_forward(void)
{
    //printf("scheduler_sort_forward()\n");
    uint32_t ind1 = 0;
    uint32_t ind2 = 1;
    for (; ind2 < SCHEDULER_TASK_COUNT; ind1++, ind2++)
    {
        if (task_queue[ind1].task != NULL && task_queue[ind2].task != NULL)
        {
            //printf("    compare times: %d vs %d\n", task_queue[ind1].next_call_ms, task_queue[ind2].next_call_ms);
            if (task_queue[ind1].next_call_ms > task_queue[ind2].next_call_ms)
            {
                //printf("    swap\n");
                scheduler_item_t tmp_item2 = task_queue[ind2];
                task_queue[ind2] = task_queue[ind1];
                task_queue[ind1] = tmp_item2;
                //printf("    after swap: %d vs %d\n", task_queue[ind1].next_call_ms, task_queue[ind2].next_call_ms);
            }
        }
    }
}

static void scheduler_sort_backward(void)
{
    //printf("scheduler_sort_backward()\n");
    uint32_t ind1 = SCHEDULER_TASK_COUNT-2;
    uint32_t ind2 = SCHEDULER_TASK_COUNT-1;
    for (; ind2 > 0; ind1--, ind2--)
    {
        if (task_queue[ind1].task != NULL && task_queue[ind2].task != NULL)
        {
            //printf("    compare times: %d vs %d\n", task_queue[ind1].next_call_ms, task_queue[ind2].next_call_ms);
            if (task_queue[ind1].next_call_ms > task_queue[ind2].next_call_ms)
            {
                //printf("    swap\n");
                scheduler_item_t tmp_item2 = task_queue[ind2];
                task_queue[ind2] = task_queue[ind1];
                task_queue[ind1] = tmp_item2;
                //printf("    after swap: %d vs %d\n", task_queue[ind1].next_call_ms, task_queue[ind2].next_call_ms);
            }
        }
    }
}

void scheduler_remove_first_task(void)
{
    uint32_t ind1 = 0;
    uint32_t ind2 = 1;
    for (; ind2 < SCHEDULER_TASK_COUNT; ind1++, ind2++)
    {
        if (task_queue[ind1].task != NULL || task_queue[ind2].task != NULL)
        {
            //printf("    move %d to %d\n", ind2, ind1);
            task_queue[ind1] = task_queue[ind2];
        }
    }
}


void scheduler_add_task(scheduler_task_func task, uint32_t call_after_ms)
{
    // Find first free slot
    uint32_t index = 0;
    while (task_queue[index].task != NULL && index < SCHEDULER_TASK_COUNT)
    {
        index++;
    }
    assert(index < SCHEDULER_TASK_COUNT); // Assert might not be good for embedded

    task_queue[index].next_call_ms = scheduler_current_time_ms + call_after_ms;
    task_queue[index].task = task;

    //printf("scheduler_add_task reordering:\n");
    //scheduler_print_table();

    scheduler_sort_backward(); // TODO should do backward, when queue has more than 2 items
    //scheduler_print_table();
}

uint32_t scheduler_loop(uint32_t current_time_ms)
{
    uint32_t next_run_ms = 0;

    scheduler_current_time_ms = current_time_ms;
    if (task_queue[0].task)
    {
        if (task_queue[0].next_call_ms <= scheduler_current_time_ms)
        {
            //printf("scheduler_loop(): running task: %p\n", task_queue[0].task);
            // Call the task
            // TODO add lock, to keep first item untouched, if task itself adds new task
            uint32_t current_task_next_run_ms = task_queue[0].task();
            if (current_task_next_run_ms == 0)
            {
                // Remove task
                scheduler_remove_first_task();
            }
            else
            {
                task_queue[0].next_call_ms = scheduler_current_time_ms + current_task_next_run_ms;
                // Sort tasks
                scheduler_sort_forward(); // forward direction
            }
        }

        // How much is left for next call
        if (task_queue[0].task)
        {
            // Calculate next_run_ms only when current time has not yet passed, else return zero.
            if (task_queue[0].next_call_ms >= scheduler_current_time_ms)
            {
                next_run_ms = task_queue[0].next_call_ms - scheduler_current_time_ms;
            }
        }
    }
    return next_run_ms;
}
