#ifndef SCREEN_H
#define SCREEN_H

//#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const char * (*screen_func)(void);

void screen_init(void);

int screen_get_count(void);

void screen_add_screen(screen_func screen);

const char * screen_get_screen(int screen_index);

#ifdef __cplusplus
}
#endif

#endif // SCREEN_H
