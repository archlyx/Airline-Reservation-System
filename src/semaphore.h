#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

/* mutex: lock
 * condition variable: notify */
typedef struct m_sem_t {
  int value;
  pthread_mutex_t lock;
  pthread_cond_t notify;
} m_sem_t;

void sem_init(m_sem_t *s, int value);
int sem_wait(m_sem_t *s);
int sem_post(m_sem_t *s);

#endif
