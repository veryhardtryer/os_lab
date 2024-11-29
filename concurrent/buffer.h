#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>

typedef struct Buffer {
  void* head;
  void* tail;
  size_t size;

  void* mem;

  size_t data_size;
  size_t max_data_count;
} Buffer;

void BufferInit(Buffer* b, size_t data_size, size_t max_data_count) {
  assert(b != 0); 
  b->data_size = data_size;
  b->max_data_count = max_data_count;
  b->mem = malloc(data_size * max_data_count);
  b->head = b->mem;
  b->tail = b->head;
  b->size = 0;
}

bool BufferEmpty(Buffer* b) {
  assert(b != 0); 
  return b->size == 0;
}

bool BufferPushBack(Buffer* b, void* data) {
  assert(b != 0); 
  if (b->size + 1 >= b->max_data_count)
    return false;

  memcpy(b->tail, data, b->data_size);
  b->size += 1;
  b->tail += b->data_size;
  return true;
}

bool BufferPopFront(Buffer* b, void* out) {
  assert(b != 0); 
  if (BufferEmpty(b))
    return false;
  
  memcpy(out, b->head, b->data_size);
  for (size_t i = 0; i < b->size - 1; ++i) {
    memcpy(b->head + (i * b->data_size), b->head + ((i+1) * b->data_size), b->data_size);
  }
  b->tail -= b->data_size;
  b->size -= 1;
  return true;
}

