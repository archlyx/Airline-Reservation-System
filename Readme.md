# Airline Reservation System #
Project #3, EECS343 Operating System

## DESIGN ##

### Thread pool ###
The thread pool is created when the process is started. The number of available worker threads are placed in pool and waiting for tasks. 

### Task queue ###
If the task queue is not empty, the task at the head of the queue will be assigned to one of the threads. When the server has received requests from users, the request will be put in the tail of the task queue. Since the size of the queue is limited as `task_queue_size_limit`, I assign a dynamic array to save the tasks and use a cirular index to manage the head & tail positions.

### Mutual Exclusion & Condition Variable ###
The `pthread_mutex_*` and `pthread_cond_*` APIs are used to protect the critical sections including the task queue and the seats, and deal with the race condition. The seats are locked when the information of the seats are requested.

### Standby List ###
To implement the standby list, we need to monitor the status of the seats. If the there is no available seat when a booking request comes, this request will be placed in a standby list. If a cancellation request is finished and detected, the thread will go to the standby list and grab the standby task to finish. To control the standby list, a binary semaphore is implemented using the `pthread_mutex_*` and `pthread_cond_*` APIs.

