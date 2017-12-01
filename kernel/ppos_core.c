#include "ppos.h"
#include "ppos_data.h"
#include <stdio.h> 
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

int tid_counter; // number of tasks, used for task id
int atomic;
unsigned int ticks_counter;

task_t *actual; // actual task
task_t main_t; // task for main function
task_t dispatcher_t; // task for dispatcher function

queue_t *ready_queue; // ready tasks queue
queue_t *sleeping_queue; // sleeping tasks queue

struct sigaction action_timer; // structure to register an interrupt sa_handler
struct itimerval timer; // timer for interruptions

void time_interrupt_handler (int signal) {
    ++ticks_counter;
    ++(actual->proc_time);
    // if it is not using an atomic instruction, 
    // it is not the dispatcher and the quantum time is over,
    // it leaves the CPU
    if (!atomic && --(actual->quantum) == 0 && actual != &dispatcher_t) {
        task_yield();
    }
}

unsigned int systime () {
    return ticks_counter;
}

task_t *scheduler() {
//#ifdef DEBUG
//    printf("scheduler: starting. ready_queue size is: %d.\n", queue_size(ready_queue));
//#endif
    if (queue_size(ready_queue) == 0)
        return NULL;

    // task of the current position in list
    task_t *actual_list = (task_t *) ready_queue->prev;

    // next task to win the CPU
    task_t *next_task = actual_list;

    do { // scrolls through the list
        actual_list = (task_t *) actual_list->next;

        // If the priority is less than the minimum possible value, 
        // the minimum remains. Otherwise, it decreases the priority value. 
        if ((actual_list->priority_dynamic+=TASK_AGING) < PRIORITY_NUM_MIN)
            actual_list->priority_dynamic = PRIORITY_NUM_MIN;

        // if the actual_list task have more priority than the next one
        if (actual_list->priority_dynamic < next_task->priority_dynamic)
            next_task = actual_list;
    } while (actual_list != ((task_t *) ready_queue->prev));

    // the selected task is removed from the ready queue
    // and has its priority reseted
    queue_remove(&ready_queue, (queue_t *) next_task);
    next_task->priority_dynamic = next_task->priority_static;

#ifdef DEBUG
    printf("scheduler: the id of the selected task is %d and the priority is %d\n.", next_task->id, next_task->priority_dynamic);
#endif
    
    return next_task;
}

void dispatcher() {
#ifdef DEBUG
    printf("dispatcher: starting.\n");
#endif
    task_t* next_task = NULL;
    // while exists tasks waiting in ready/sleeping queue
    while ((next_task = scheduler()) || (queue_size(sleeping_queue) > 0)) {
        if (next_task) {
            next_task->state = RUNNING;

            task_switch(next_task);

            // if task is finished, free its stack
            if (next_task->state == FINISHED) {
                free(next_task->context.uc_stack.ss_sp);
            }
        }

        if(queue_size(sleeping_queue) > 0) {
            queue_t *aux, *next = NULL;
            for (aux = sleeping_queue->next; aux != sleeping_queue; aux = next) {
                next = aux->next;
                if ( (((task_t *) aux)->state == SLEEPING) && ((task_t *) aux)->wakeUpTime <= systime() ) {
                    queue_remove(&sleeping_queue, (queue_t *) aux);
                    ((task_t *) aux)->state = READY;
                    queue_append(&ready_queue, (queue_t *) aux);
                }
            }
            if ( (((task_t *) aux)->state == SLEEPING) && ((task_t *) aux)->wakeUpTime <= systime() ) {
                queue_remove(&sleeping_queue, (queue_t *) aux);
                ((task_t *) aux)->state = READY;
                queue_append(&ready_queue, (queue_t *) aux);
            }

        }
    }

    task_exit(0);
}

void ppos_init () {
#ifdef DEBUG
    printf ("ppos_init: starting\n") ;
#endif

    /* disables the stdout buffer */
    setvbuf (stdout, 0, _IONBF, 0) ;

    ready_queue = NULL;
    sleeping_queue = NULL;

    /* sets the parameters of the main task */
    main_t.prev = NULL;
    main_t.next = NULL;
    main_t.id = 0;
    main_t.priority_static = PRIORITY_DEFAULT;
    main_t.priority_dynamic = PRIORITY_DEFAULT;
    main_t.state = READY;
    main_t.quantum = QUANTUM;
    main_t.exec_time = systime();
    main_t.proc_time = 0;
    main_t.activations = 0;

    tid_counter = 1;

    ticks_counter = 0;

    // register an interrupt handler
    action_timer.sa_handler = time_interrupt_handler;
    sigemptyset (&action_timer.sa_mask);
    action_timer.sa_flags = 0;
    if (sigaction (SIGALRM, &action_timer, 0) < 0) {
        perror ("Error in sigaction: ") ;
        exit (1) ;
    }

    // set timer
    timer.it_value.tv_usec = TICK_TIMER_US;        // microseconds
    timer.it_value.tv_sec = TICK_TIMER_S;          // seconds
    timer.it_interval.tv_usec = TICK_TIMER_US;     // microseconds
    timer.it_interval.tv_sec = TICK_TIMER_S;       // seconds

    if (setitimer (ITIMER_REAL, &timer, 0) < 0) {
        perror ("Error in setitimer: ") ;
        exit (1) ;
    }

    // creates dispatcher, but it never goes to ready queue
    task_create(&dispatcher_t, dispatcher, NULL);
    queue_remove(&ready_queue, (queue_t *) &dispatcher_t);

    actual = &main_t;
    atomic = 0; // can be preempted

    task_yield();
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {
#ifdef DEBUG
    printf ("task_create: starting\n") ;
#endif
    // allocates memory for the task stack 
    char *stack;
    stack = malloc(STACKSIZE);
    
    if(stack) {
        task->prev = NULL;
        task->next = NULL;
        task->id = tid_counter++;

        getcontext(&(task->context));

        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;

        makecontext(&(task->context), (void *) start_func, 1, arg);

        task->stack = stack;
        task->priority_static = PRIORITY_DEFAULT;
        task->priority_dynamic = PRIORITY_DEFAULT;
        task->state = READY;
        task->quantum = QUANTUM;
        task->exec_time = systime();
        task->proc_time = 0;
        task->activations = 0;

        #ifdef DEBUG
            printf ("task_create: task created %d\n", task->id);
        #endif

        queue_append(&ready_queue, (queue_t *) task);

        return task->id;
    }
    else {
        perror ("task_create: can not create task");
        return -1;
    }
}

void task_exit (int exitCode) {
#ifdef DEBUG
    printf ("task_exit: starting\n") ;
#endif
    actual->state = FINISHED;
    actual->exec_time = systime() - actual->exec_time; 
    actual->exit_code = exitCode;
    printf("Task %d exit: execution time %u ms, processor time %u ms, %u activations\n", 
            actual->id, actual->exec_time, actual->proc_time, actual->activations);

    if (actual->id == dispatcher_t.id) { // if the dispatcher is terminating, it returns the control to main
        free(dispatcher_t.context.uc_stack.ss_sp);
    } else {
        // wakes up the tasks that were waiting for the actual task to finish
        task_t *aux = NULL;
        while(queue_size(actual->join_tasks) > 0) {
            aux = (task_t *) queue_remove(&(actual->join_tasks), (queue_t *) actual->join_tasks);
            queue_append(&ready_queue, (queue_t *) aux);
            aux->id = READY;
        }

        task_switch(&dispatcher_t); // return control to dispatcher
    }
}

int task_switch (task_t *task) {
#ifdef DEBUG
    printf ("task_switch: starting.\nactual.id: %d\ntask.id: %d\n", actual->id, task->id) ;
#endif
    // save the actual task as backup and make the task (parameter) the actual
    task_t *backup = actual;
    actual = task;
    actual->quantum = QUANTUM;
    actual->priority_dynamic = actual->priority_static;
    ++(actual->activations);

    if (swapcontext(&(backup->context), &(task->context)) < 0) {
        fprintf(stderr, "task_switch: can not switch contexts\n");
        
        actual = backup;
        actual->state = RUNNING;

        task->state = READY;
        queue_append(&ready_queue, (queue_t *) task);

        return -1;
    }

    return 0;
}

int task_id() {
#ifdef DEBUG
    printf("task_id: actual task id is %d\n", actual->id);
#endif
    return actual->id;
}

void task_setprio(task_t *task, int prio) {
#ifdef DEBUG
    printf("task_setprio: task id is %d and priority of the task is %d\n", ((!task) ? actual->id : task->id), prio);
#endif
    if (prio >= PRIORITY_NUM_MIN && prio <= PRIORITY_NUM_MAX) {
        if (task) {
            task->priority_static = prio;
            task->priority_dynamic = prio;
        } else {
            actual->priority_static = prio;
            actual->priority_dynamic = prio;
        }
    }
}

int task_getprio(task_t *task) {
#ifdef DEBUG
    printf("task_getprio: task id is %d and priority of the task is %d\n", 
            ((!task) ? actual->id : task->id), ((!task) ? actual->priority_static : task->priority_static));
#endif    
    return (task ? task->priority_static : actual->priority_static);
}

void task_yield() {
#ifdef DEBUG
    printf("task_yield: starting, yielding task id is %d\n", actual->id);
#endif
    queue_append(&ready_queue, (queue_t *) actual);
    actual->state = READY;
    task_switch(&dispatcher_t);
}

int task_join (task_t *task) {
#ifdef DEBUG
    printf("task_join: starting, joined task id is %d \n", actual->id);
#endif
    if(!task || task->state == FINISHED)
        return -1;

    // suspends the task and places it in the join queue
    queue_append(&(task->join_tasks), (queue_t *) actual);
    actual->state = SUSPENDED;

    task_switch(&dispatcher_t);

    // when the task returns, it means that the expected
    // task has already finished and we have the code
    return task->exit_code;
}

void task_sleep (int t) {
#ifdef DEBUG
    printf("task_sleep: starting, the id of the sleeping task is %d \n", actual->id);
#endif
    // remove the task from the ready queue (mas não está na fila de prontas...)
    // queue_remove(&ready_queue, (queue_t *) actual);

    // calculates at what time this task should be agreed
    actual->wakeUpTime = systime()+t;
    // put to sleep the task and places it in the sleeping queue
    actual->state = SLEEPING;
    queue_append(&sleeping_queue, (queue_t *) actual);

    task_switch(&dispatcher_t);
}

// semaphore

int sem_create (semaphore_t *s, int value) {
#ifdef DEBUG
    printf("sem_create: starting.\n");
#endif
    if(!s)
        return -1;

    atomic = 1; // cant be preempted

    s->count = value;
    s->queue = NULL;
    s->state = READY;

    atomic = 0; // can be preempted
    return 0;
}

int sem_down (semaphore_t *s) {
#ifdef DEBUG
    printf("sem_down: starting.\n");
#endif
    if(!s || s->state == DESTROYED)
        return -1;

    atomic = 1; // cant be preempted
    
    --(s->count);
    if(s->count < 0) {
        queue_append(&(s->queue), (queue_t *) actual);
        actual->state = SUSPENDED;

        atomic = 0;
        task_switch(&dispatcher_t);

        if(s->state == DESTROYED)
            return -1;
    }

    atomic = 0; // can be preempted
    return 0;
}

int sem_up (semaphore_t *s) {
#ifdef DEBUG
    printf("sem_up: starting.\n");
#endif
    if(!s || s->state == DESTROYED)
        return -1;

    atomic = 1; // cant be preempted

    ++(s->count);
    if(queue_size(s->queue) > 0) {
        task_t *aux = (task_t *) queue_remove(&(s->queue), s->queue);
        aux->state = READY;
        queue_append(&ready_queue, (queue_t *) aux);
    }

    atomic = 0; // can be preempted
    return 0;
}

int sem_destroy (semaphore_t *s) {
#ifdef DEBUG
    printf("sem_destroy: starting.\n");
#endif
    if(!s || s->state == DESTROYED)
        return -1;

    atomic = 1; // cant be preempted

    while(queue_size(s->queue) > 0) {
        task_t *aux = (task_t *) queue_remove(&(s->queue), s->queue);
        aux->state = READY;
        aux->exit_code = -1;
        queue_append(&ready_queue, (queue_t *) aux);
    }
    
    s->state = DESTROYED;

    atomic = 0; // can be preempted
    return 0;
}

int mqueue_create (mqueue_t *queue, int max, int size) {
#ifdef DEBUG
    printf("mqueue_create: starting.\n");
#endif
	if(!queue)
		return -1;
	// inicializes semaphores, allocates the message queue and initializes attributes
	if(sem_create(&(queue->semaphore_acess), 1) == -1 || 
	   sem_create(&(queue->semaphore_items), 0) == -1 ||
	   sem_create(&(queue->semaphore_vacancies), max) == -1)
		return -1;

	if(!(queue->buffer = malloc(max*size)))
		return -1;

	queue->head = 0;
	queue->tail = 0;
	queue->queue_size = 0;
  	queue->queue_capacity = max;
	queue->msg_capacity = size;
	queue->state = READY;

	return 0;
}

int mqueue_send (mqueue_t *queue, void *msg) {
#ifdef DEBUG
    printf("mqueue_send: starting.\n");
#endif
	if(!queue || queue->state == DESTROYED)
		return -1;

	// first needs a vacancy, then needs exclusive access to the buffer
	if(sem_down(&(queue->semaphore_vacancies)) == -1 || sem_down(&(queue->semaphore_acess)) == -1)
		return -1;

	// copy message to buffer
	memcpy(queue->buffer+(queue->tail*queue->msg_capacity), msg, queue->msg_capacity);
	queue->tail = (queue->tail+1)%queue->queue_capacity;
	++queue->queue_size;

	// releases exclusive access to the buffer and increments items
	if(sem_up(&(queue->semaphore_acess)) == -1 || sem_up(&(queue->semaphore_items)) == -1)
		return -1;

	return 0;
}

int mqueue_recv (mqueue_t *queue, void *msg) {
#ifdef DEBUG
    printf("mqueue_recv: starting.\n");
#endif
    if(!queue || queue->state == DESTROYED)
		return -1;

	// first needs an item, then needs exclusive access to the buffer
	if(sem_down(&(queue->semaphore_items)) == -1 || sem_down(&(queue->semaphore_acess)) == -1)
		return -1;

	// copy buffer to message
	memcpy(msg, queue->buffer+(queue->head*queue->msg_capacity), queue->msg_capacity);
	queue->head = (queue->head+1)%queue->queue_capacity;
	--queue->queue_size;

	// releases exclusive access to the buffer and increments vacancies
	if(sem_up(&(queue->semaphore_acess)) == -1 || sem_up(&(queue->semaphore_vacancies)) == -1)
		return -1;

	return 0;
}

int mqueue_destroy (mqueue_t *queue) {
#ifdef DEBUG
    printf("mqueue_destroy: starting.\n");
#endif
    if(!queue || queue->state == DESTROYED)
		return -1;

	free(queue->buffer);
	sem_destroy(&(queue->semaphore_acess));
	sem_destroy(&(queue->semaphore_items));
	sem_destroy(&(queue->semaphore_vacancies));
	queue->state = DESTROYED;

	return 0;
}

int mqueue_msgs (mqueue_t *queue) {
#ifdef DEBUG
    printf("mqueue_msgs: starting.\n");
#endif
    if(!queue || queue->state == DESTROYED)
		return -1;

	return queue->queue_size;
}