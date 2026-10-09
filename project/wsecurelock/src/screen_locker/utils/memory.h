#pragma once

#include <stddef.h>

int sl_memory_memfile_create(const char* name, size_t size);
void sl_memory_memfile_close(int fd);
