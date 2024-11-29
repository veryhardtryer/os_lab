#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct SharedBuffer {
  bool valid;
  bool is_close;
  pthread_cond_t is_empty;
  pthread_mutex_t mutex;

  int data;
} SharedBuffer;

static SharedBuffer shared_buffer;

void Init() {
  pthread_mutex_init(&shared_buffer.mutex, 0);
  pthread_cond_init(&shared_buffer.is_empty, 0);
  shared_buffer.valid = false;
}

void Close() {
  pthread_mutex_lock(&shared_buffer.mutex);
  shared_buffer.is_close = true;
  pthread_cond_broadcast(&shared_buffer.is_empty);
  pthread_mutex_unlock(&shared_buffer.mutex);
}

bool Send(int data) {
  pthread_mutex_lock(&shared_buffer.mutex);
  if (shared_buffer.is_close) {
    pthread_mutex_unlock(&shared_buffer.mutex);
    return false;
  }
  shared_buffer.data = data;
  shared_buffer.valid = true;
  pthread_cond_signal(&shared_buffer.is_empty);
  pthread_mutex_unlock(&shared_buffer.mutex);
  return true;
}

bool Receive(int* data) {
  pthread_mutex_lock(&shared_buffer.mutex);
  while(!shared_buffer.valid) {
    if (shared_buffer.is_close) {
      pthread_mutex_unlock(&shared_buffer.mutex);
      return false;
    }
    pthread_cond_wait(&shared_buffer.is_empty, &shared_buffer.mutex);
  }
  *data = shared_buffer.data; 
  shared_buffer.valid = false;
  pthread_mutex_unlock(&shared_buffer.mutex);
  return true;
}

void* ThreadRunLoop(void* data) {
  int id = *(int*)(data);
  while (true) {
    int data;

    if (!Receive(&data)) {
      break;
    }

    printf("Thread(%d) receive data [ %d ]\n", id, data);
  }
  printf("Thread(%d) shutdown\n", id);
  return 0;
}

int main() {

  Init();

  pthread_t t;
  int t_id = 1;

  pthread_create(&t, 0, ThreadRunLoop, &t_id);

  sleep(1);
  for (int i = 0; i < 10; ++i) {
    printf("Send main thread [ %d ]\n", i);
    Send(i);
    sleep(1);
  }

  Close();

  pthread_join(t, 0);
  return 0;
}
