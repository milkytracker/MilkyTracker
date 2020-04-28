#include <stdlib.h>

void * operator new(size_t sz)
{
    return malloc(sz);
}

void operator delete(void * p)
{
    free(p);
}

void operator delete(void * p, size_t sz)
{
    free(p);
}

void *operator new[](size_t sz)
{
    return malloc(sz);
}

void operator delete[](void * p)
{
    free(p);
}
