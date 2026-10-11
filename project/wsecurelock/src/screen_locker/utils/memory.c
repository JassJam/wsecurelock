#define _GNU_SOURCE
#include "memory.h"

#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

void sl_memory_zero(void* data, size_t size)
{
    explicit_bzero(data, size);
}

void sl_memory_lock(void* data, size_t size)
{
    mlock(data, size);
}

void sl_memory_unlock(void* data, size_t size)
{
    munlock(data, size);
}

int sl_memory_memfile_create(const char* name, size_t size)
{
    int fd = memfd_create(name, MFD_CLOEXEC);
    if (fd < 0 || ftruncate(fd, size) < 0)
    {
        if (fd >= 0)
        {
            sl_memory_memfile_close(fd);
        }
    }
    return fd;
}

void sl_memory_memfile_close(int fd)
{
    close(fd);
}
