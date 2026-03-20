#include "screen.h"
#include "stddef.h" // NULL
//#include "assert.h"
//#include <stdio.h>

#define SCREEN_COUNT_MAX 5

static screen_func screen_list[SCREEN_COUNT_MAX] = { 0 };


void screen_init(void)
{
    for (int i = 0; i<SCREEN_COUNT_MAX; ++i)
    {
        screen_list[i] = NULL;
    }
}

int screen_get_count(void)
{
    int index = 0;
    while (screen_list[index] != NULL)
    {
        ++index;
    }
    return index;
}

void screen_add_screen(screen_func screen)
{
    int next_free_slot = screen_get_count();
    screen_list[next_free_slot] = screen;
}

const char * screen_get_screen(int screen_index)
{
    if (screen_index < screen_get_count())
    {
        return screen_list[screen_index]();
    }
    return "Not found";
}
