#include "content.h"
#include "stddef.h" // NULL
//#include "assert.h"
//#include <stdio.h>

#define CONTENT_PAGE_COUNT_MAX 10

static content_func content_page_list[CONTENT_PAGE_COUNT_MAX] = { 0 };


void content_init(void)
{
    for (int i = 0; i<CONTENT_PAGE_COUNT_MAX; ++i)
    {
        content_page_list[i] = NULL;
    }
}

int content_get_page_count(void)
{
    int index = 0;
    while (content_page_list[index] != NULL)
    {
        ++index;
    }
    return index;
}

void content_add_page(content_func page)
{
    int next_free_slot = content_get_page_count();
    content_page_list[next_free_slot] = page;
}

const char * content_get_page(int page_index)
{
    if (page_index < content_get_page_count())
    {
        return content_page_list[page_index]();
    }
    return "Not found";
}
