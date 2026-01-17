#include "unity.h"
#include <scheduler.h>
//#include <stdio.h>

uint32_t call_counter__task_interval_3 = 0;
uint32_t call_counter__task_interval_5 = 0;
uint32_t call_counter__task_interval_7 = 0;
uint32_t call_counter__task_once = 0;


void setUp(void)
{
    // set stuff up here
    scheduler_clear_all();
    call_counter__task_interval_3 = 0;
    call_counter__task_interval_5 = 0;
    call_counter__task_interval_7 = 0;
    call_counter__task_once = 0;
}

void tearDown(void)
{
    // clean stuff up here
}

// =========================
// Task functions
// =========================
int task_interval_3(void)
{
    //printf("    -> task_interval_3: %p\n", task_interval_3);
    call_counter__task_interval_3++;
    return 3;
}

int task_interval_5(void)
{
    //printf("    -> task_interval_5: %p\n", task_interval_5);
    call_counter__task_interval_5++;
    return 5;
}

int task_interval_7(void)
{
    //printf("    -> task_interval_7: %p\n", task_interval_3);
    call_counter__task_interval_7++;
    return 7;
}


int task_once(void)
{
    //printf("    -> task_once: %p\n", task_once);
    call_counter__task_once++;
    return 0;
}

// =========================
// Test cases
// =========================

void test_scheduler_add_task(void)
{
    scheduler_add_task(task_interval_5, 0);
}

void test_scheduler_loop__one_task(void)
{
    uint32_t time = 0;
    uint32_t next_time = 0;
    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(0, next_time);

    scheduler_add_task(task_interval_5, 0);
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5); // Not called yet

    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5); // Called first time
    TEST_ASSERT_EQUAL(5, next_time);

    time += next_time;
    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_5); // Called first time
    TEST_ASSERT_EQUAL(5, next_time);

    time += 2; // Shorter time than wanted
    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(3, next_time);

    time += 2; // Shorter time than wanted
    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(1, next_time);

    time += 1;
    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(3, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(5, next_time);
}

void test_scheduler_loop__two_tasks(void)
{
    uint32_t time = 0;
    uint32_t next_time = 0;
    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(0, next_time);

    scheduler_add_task(task_interval_5, 10); // calls: 10, 15, 20, 25
    scheduler_add_task(task_interval_7, 2); // calls: 2, 9, 16, 23
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_7); // Not called yet

    next_time = scheduler_loop(time); // time 0
    TEST_ASSERT_EQUAL(2, next_time);
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_7); // Not called yet

    time += next_time;
    next_time = scheduler_loop(time); // time 2
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_7);
    TEST_ASSERT_EQUAL(7, next_time);

    time += next_time;
    next_time = scheduler_loop(time); // time 9
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_7);
    TEST_ASSERT_EQUAL(1, next_time);

    time += next_time;
    next_time = scheduler_loop(time); // time 10
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_7);
    TEST_ASSERT_EQUAL(5, next_time);

    time += next_time;
    next_time = scheduler_loop(time); // time 15
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_7);
    TEST_ASSERT_EQUAL(1, next_time);

    time += next_time;
    next_time = scheduler_loop(time); // 16
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(3, call_counter__task_interval_7);
    TEST_ASSERT_EQUAL(4, next_time);
}

void test_scheduler_loop__one_shot_task(void)
{
    uint32_t time = 0;
    uint32_t next_time = 0;
    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(0, next_time);

    scheduler_add_task(task_interval_5, 1); // calls: 1, 6, 11
    scheduler_add_task(task_once, 2); // calls: 2
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(0, call_counter__task_once); // Not called yet

    next_time = scheduler_loop(time); // time 0
    TEST_ASSERT_EQUAL(1, next_time);
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(0, call_counter__task_once); // Not called yet

    time += next_time;
    next_time = scheduler_loop(time); // time 1
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(0, call_counter__task_once);
    TEST_ASSERT_EQUAL(1, next_time);

    time += next_time;
    next_time = scheduler_loop(time); // time 2
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(1, call_counter__task_once);
    TEST_ASSERT_EQUAL(4, next_time);

    time += next_time;
    next_time = scheduler_loop(time); // time 6
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(1, call_counter__task_once);
    TEST_ASSERT_EQUAL(5, next_time);
}

void test_scheduler_loop__three_tasks(void)
{
    uint32_t time = 0;
    uint32_t next_time = 0;
    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(0, next_time);

    scheduler_add_task(task_interval_7, 2); // calls: 2, 9, 16
    scheduler_add_task(task_interval_5, 1); // calls: 1, 6, 11, 16
    scheduler_add_task(task_interval_3, 0); // calls: 0, 3, 6, 9, 12
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_3); // Not called yet
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_7); // Not called yet

    next_time = scheduler_loop(time); // time 0
    TEST_ASSERT_EQUAL(1, next_time);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 1
    TEST_ASSERT_EQUAL(1, next_time);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 2
    TEST_ASSERT_EQUAL(1, next_time);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 3
    TEST_ASSERT_EQUAL(3, next_time);
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 6
    TEST_ASSERT_EQUAL(0, next_time);
    TEST_ASSERT_EQUAL(3, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 6
    TEST_ASSERT_EQUAL(3, next_time);
    TEST_ASSERT_EQUAL(3, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 9
    TEST_ASSERT_EQUAL(0, next_time);
    TEST_ASSERT_EQUAL(4, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 9
    TEST_ASSERT_EQUAL(2, next_time);
    TEST_ASSERT_EQUAL(4, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_7);
}

void test_scheduler_loop__called_late(void)
{
    uint32_t time = 0;
    uint32_t next_time = 0;
    next_time = scheduler_loop(time);
    TEST_ASSERT_EQUAL(0, next_time);

    scheduler_add_task(task_interval_7, 2); // calls: 2, 2+9=11
    scheduler_add_task(task_interval_5, 1); // calls: 1, 2+6=7
    scheduler_add_task(task_interval_3, 0); // calls: 0, 2+3=5
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_3); // Not called yet
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5); // Not called yet
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_7); // Not called yet

    time = 2;
    next_time = scheduler_loop(time); // time 2
    TEST_ASSERT_EQUAL(0, next_time);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 2
    TEST_ASSERT_EQUAL(0, next_time);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(0, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 2
    TEST_ASSERT_EQUAL(3, next_time);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_7);

    time += next_time;
    next_time = scheduler_loop(time); // time 5
    TEST_ASSERT_EQUAL(2, next_time);
    TEST_ASSERT_EQUAL(2, call_counter__task_interval_3);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_5);
    TEST_ASSERT_EQUAL(1, call_counter__task_interval_7);
}


// TODO tests
// - task add itself new task to scheduler -> verify that tasks does not break, when updating table during execution

int runUnityTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_scheduler_add_task);
    RUN_TEST(test_scheduler_loop__one_task);
    RUN_TEST(test_scheduler_loop__two_tasks);
    RUN_TEST(test_scheduler_loop__one_shot_task);
    RUN_TEST(test_scheduler_loop__three_tasks);
    RUN_TEST(test_scheduler_loop__called_late);
    return UNITY_END();
}


#ifdef PLATFORM_NATIVE
/**
  * For native dev-platform or for some embedded frameworks
  */
int main(void) {
  return runUnityTests();
}

#else

/**
 * For Arduino framework
 */
void setup()
{
    // Wait ~2 seconds before the Unity test runner
    // establishes connection with a board Serial interface
    delay(2000);

    runUnityTests();
}
void loop() {}
#endif
