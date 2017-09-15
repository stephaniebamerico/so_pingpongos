#include "ppos.h"
#include "ppos_data.h"
#include <stdio.h> 
#include <stdlib.h>

int tid_counter; // number of tasks, used for task id

task_t *actual; // actual task
task_t main_t; // task for main function
task_t dispatcher_t; // task for dispatcher function

queue_t *ready_queue; // ready tasks queue

task_t *scheduler() {
#ifdef DEBUG
    printf("scheduler: starting.\n");
#endif
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
    queue_remove(&ready_queue, (queue_t *) &main_t);

    task_t* next_task = NULL;
    while ((next_task = scheduler())) { // while exists tasks waiting in the queue
        if (next_task) {
            next_task->state = RUNNING;

            task_switch(next_task);

            // if task is finished, free its stack
            if (next_task->state == FINISHED) {
                free(next_task->context.uc_stack.ss_sp);
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

    /* sets the parameters of the main task */
    main_t.prev = NULL;
    main_t.next = NULL;
    main_t.id = 0;
    main_t.priority_static = PRIORITY_DEFAULT;
    main_t.priority_dynamic = PRIORITY_DEFAULT;
    main_t.state = READY;

    tid_counter = 1;

    actual = &main_t;

    // creates dispatcher, but it never goes to ready queue
    task_create(&dispatcher_t, dispatcher, NULL);
    queue_remove(&ready_queue, (queue_t *) &dispatcher_t);
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

    if (actual->id == dispatcher_t.id) { // if the dispatcher is terminating, it returns the control to main
        free(dispatcher_t.context.uc_stack.ss_sp);
        task_switch(&main_t);
    } else 
        task_switch(&dispatcher_t); // return control to dispatcher
}

int task_switch (task_t *task) {
#ifdef DEBUG
    printf ("task_switch: starting.\nactual.id: %d\ntask.id: %d\n", actual->id, task->id) ;
#endif
    // save the actual task as backup and make the task (parameter) the actual
    task_t *backup = actual;
    actual = task;

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
