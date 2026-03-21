#ifndef CONTENT_H
#define CONTENT_H

//#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const char * (*content_func)(void);

void content_init(void);

int content_get_page_count(void);

void content_add_page(content_func page);

const char * content_get_page(int page_index);

#ifdef __cplusplus
}
#endif

#endif // CONTENT_H
