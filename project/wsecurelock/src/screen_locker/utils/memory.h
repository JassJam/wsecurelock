#pragma once

#include <stddef.h>

void sl_memory_zero(void* data, size_t size);

void sl_memory_lock_all();
void sl_memory_unlock_all();

void sl_memory_lock(void* data, size_t size);
void sl_memory_unlock(void* data, size_t size);

int sl_memory_memfile_create(const char* name, size_t size);
void sl_memory_memfile_close(int fd);
