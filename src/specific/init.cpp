#include "standard.h"
#include "specific.h"

extern "C" {

extern char *malloc_buffer;
extern int malloc_size;
extern char *malloc_ptr;
extern int malloc_free;
extern int malloc_used;

typedef struct watertab {
    char shimmer;
    char choppy;
    unsigned char random;
    unsigned char abs;
} WATERTAB;

unsigned short GetRandom(WATERTAB *w, int n)
{
    int i;
    unsigned short random;

    do {
        random = rand() & 0xfc;
        for (i = 0; i < n; i++) {
            if (w[i].random == random)
                break;
        }
        if (i == n)
            return random;
    } while (1);
    return 0;
}

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
