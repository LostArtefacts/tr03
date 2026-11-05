#include "standard.h"
#include "specific.h"

extern "C" {

extern char *malloc_buffer;
extern int malloc_size;
extern char *malloc_ptr;
extern int malloc_free;
extern int malloc_used;

void init_game_malloc(void)
{
    malloc_ptr = malloc_buffer;
    malloc_free = malloc_size;
    malloc_used = 0;
}

void game_free(int size, int type)
{
    size = (size + 3) & ~3;
    malloc_ptr -= size;
    malloc_free += size;
    malloc_used -= size;
}

}
