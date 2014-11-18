#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "semaphore.h"


/* Initialize the semaphore */
void sem_init(m_sem_t *s, int value) {
  s->value = value;
  pthread_mutex_init(&s->lock, NULL);
  pthread_cond_init(&s->notify, NULL);
}

int sem_wait(m_sem_t *s)
{
  pthread_mutex_lock(&s->lock);
  while (s->value == 0)
    pthread_cond_wait(&s->notify, &s->lock);

  s->value = 0;
  pthread_mutex_unlock(&s->lock);

  return 0;
}

int sem_post(m_sem_t *s)
{
  pthread_mutex_lock(&s->lock);
  s->value = 1;
  pthread_cond_signal(&s->notify);
  pthread_mutex_unlock(&s->lock);

  return 0;
}
