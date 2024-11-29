#pragma once

#include <pthread.h>

#include "buffer.h"

typedef struct ThreadQueue {
  Buffer storage;
  pthread_mutex_t mutex;
  pthread_cond_t is_empty;
  bool is_close; 
} ThreadQueue;

void ThreadQueueInit(ThreadQueue* q, size_t data_size, size_t max_data_count) {
  assert(q != 0);
  pthread_mutex_init(&q->mutex, 0);
  pthread_cond_init(&q->is_empty, 0);
  q->is_close = false;
  BufferInit(&q->storage, data_size, max_data_count);
}

bool ThreadQueuePut(ThreadQueue* q, void* data) {
  assert(q != 0);
  pthread_mutex_lock(&q->mutex);
  if (q->is_close) {
    pthread_mutex_unlock(&q->mutex);
    return false;
  }
  BufferPushBack(&q->storage, data);
  pthread_cond_signal(&q->is_empty);
  pthread_mutex_unlock(&q->mutex);
  return true;
}

bool ThreadQueueTake(ThreadQueue* q, void* out) {
  assert(q != 0);
  pthread_mutex_lock(&q->mutex);
  while(BufferEmpty(&q->storage)) {
    if (q->is_close) {
      pthread_mutex_unlock(&q->mutex);
      return false;
    }
    pthread_cond_wait(&q->is_empty, &q->mutex);
  }
  BufferPopFront(&q->storage, out);
  pthread_mutex_unlock(&q->mutex);
  return true;
}

void ThreadQueueClose(ThreadQueue* q) {
  assert(q != 0);
  pthread_mutex_lock(&q->mutex);
  q->is_close = true;
  pthread_cond_broadcast(&q->is_empty);
  pthread_mutex_unlock(&q->mutex);
}
