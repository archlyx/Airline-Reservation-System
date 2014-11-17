#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "thread_pool.h"

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

#define MAX_THREADS 20
#define STANDBY_SIZE 8

typedef struct {
  void (*function)(void *);
  void *argument;
} pool_task_t;


/* Here we use a circular array to save the task queue */
struct pool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  pool_task_t *queue;
  int head;
  int tail;
  int thread_count;
  int task_queue_size_limit;
  int shutdown;
};

static void *thread_do_work(void *pool);


/*
 * Create a threadpool, initialize variables, etc
 *
 */
pool_t *pool_create(int queue_size, int num_threads)
{
  int i;
  pool_t *thread_pool;

  if (num_threads == 0)
    return NULL;


  /* Assign memory for the threads in the pool */
  thread_pool = (pool_t *)malloc(sizeof(pool_t));
  thread_pool->threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));

  thread_pool->thread_count = num_threads;
  thread_pool->task_queue_size_limit = queue_size;

  /* Create the worker threads in the pool */
  for (i = 0; i < num_threads; i++) {
    pthread_create(&thread_pool->threads[i], NULL,
                   thread_do_work, (void *)thread_pool);
  }

  /* Initialize the task queue */
  thread_pool->queue = (pool_task_t *)malloc(queue_size * sizeof(pool_task_t)); 
  thread_pool->head = 0;
  thread_pool->tail = 0;

  thread_pool->shutdown = 0;

  /* Initialize the mutex & condition variable */
  pthread_mutex_init(&thread_pool->lock, NULL);
  pthread_cond_init(&thread_pool->notify, NULL);

  return thread_pool;
}


/*
 * Add a task to the threadpool
 *
 */
int pool_add_task(pool_t *pool, void (*function)(void *), void *argument)
{
  pool_task_t *task;
  /* Get the lock */
  pthread_mutex_lock(&pool->lock);

  task = &pool->queue[pool->tail];
  pool->tail = pool->tail + 1;
  pool->tail = ((pool->tail == pool->task_queue_size_limit) ? 0 : pool->tail);

  task->function = function;
  task->argument = argument;

  /* Unlock */
  pthread_cond_signal(&pool->notify);
  pthread_mutex_unlock(&pool->lock);
        
  return 0;
}



/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int pool_destroy(pool_t *pool)
{
  int i;

  if (pool == NULL)
    return -1;

  pool->shutdown = 1;

  /* Wake up all the threads */
  for (i = 0; i < pool->thread_count; i++)
    pthread_cond_signal(&pool->notify);

  for (i = 0; i < pool->thread_count; i++)
    pthread_join(pool->threads[i], NULL);

  free(pool->queue); 
  free(pool->threads); 

  free(pool);
  return 0;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *pool)
{ 
  pool_t *thread_pool = pool;
  pool_task_t *task;

  while(1) {
    /* Get the lock */
    pthread_mutex_lock(&thread_pool->lock);
    pthread_cond_wait(&thread_pool->notify, &thread_pool->lock);
    
    /* If the server shut down */
    if (thread_pool->shutdown == 1)
      break;

    task = &thread_pool->queue[thread_pool->head];
    thread_pool->head += 1;
    thread_pool->head = ((thread_pool->head == thread_pool->task_queue_size_limit) ? 0 : thread_pool->head);

    /* Do the task */
    (* task->function)(&task->argument);

    /* Unlock */
    pthread_mutex_unlock(&thread_pool->lock);
  }

  pthread_exit(NULL);
  return NULL;
}
