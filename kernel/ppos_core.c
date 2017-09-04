#include "ppos.h"
#include "ppos_data.h"
#include <stdio.h> 
#include <stdlib.h>

int tid_counter; // number of tasks, used for task id

task_t *actual; // actual task
task_t main_t; // task for main function

#define STACKSIZE 32768

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

    tid_counter = 1;

    actual = &main_t;
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
    
        #ifdef DEBUG
            printf ("task_create: task created\n");
        #endif

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

    task_switch (&main_t);
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
