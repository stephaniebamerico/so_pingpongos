#include "ppos_disk.h"
#include "hard_disk.h"
#include "ppos.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

/* local variables */
disk_t hd; // hard disk
task_t disk_manager; // task that will manage the disk
queue_t *disk_queue;
struct sigaction disk_timer; // structure to register an interrupt sa_handler

/* variables in ppos_core.c */
extern int atomic;
extern task_t *actual; // actual task
extern queue_t *ready_queue; // ready tasks queue
extern task_t dispatcher_t; // task for dispatcher function

void disk_interrupt_handler (int signal) {
	atomic = 1; // can not be preempted

    // requested operation completed
    if(signal == SIGUSR1) {
    	sem_down(&(hd.semaphore_acess));
    	// if the disk manager is sleeping, wake up
    	if(disk_manager.state == SUSPENDED) {
    		disk_manager.state = READY;
    		queue_append(&ready_queue, (queue_t *) &disk_manager);
    	}

    	hd.signal = 1;
    }

    sem_up(&(hd.semaphore_acess));

    atomic = 0;
}

void disk_driver_body (void *args) {
#ifdef DEBUG
    printf("disk_driver_body: starting.\n");
#endif
	while (1) {
		// obtém o semáforo de acesso ao disco
		sem_down(&(hd.semaphore_acess));

		// se foi acordado devido a um sinal do disco
		if(hd.signal == 1) {
			hd.signal = 0;

			// acorda a tarefa cujo pedido foi atendido
			task_request *requester_t = (task_request *) disk_queue;
			queue_remove(&(disk_queue), disk_queue);

			requester_t->requester->state = READY;
			queue_append(&ready_queue, (queue_t *) requester_t->requester);

			free(requester_t);
		}

		// se o disco estiver livre e houver pedidos de E/S na fila
		if (disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE && queue_size(disk_queue) > 0) {
			// escolhe na fila o pedido a ser atendido, usando FCFS
			task_request *request_t = (task_request *) disk_queue;

			// solicita ao disco a operação de E/S, usando disk_cmd()
			disk_cmd(request_t->request, request_t->block, request_t->buffer);
		}

		// libera o semáforo de acesso ao disco
		sem_up(&(hd.semaphore_acess));

		// suspende a tarefa corrente (retorna ao dispatcher)
		actual->state = SUSPENDED;
		
		task_switch(&dispatcher_t);
   }
}

int disk_mgr_init (int *numBlocks, int *blockSize) {
#ifdef DEBUG
    printf("disk_mgr_init: starting. numBlocks: %d blockSize: %d.\n", *numBlocks, *blockSize);
#endif
    disk_queue = NULL;

    /* interrupt handler */
    disk_timer.sa_handler = disk_interrupt_handler;
    sigemptyset (&(disk_timer.sa_mask));
    disk_timer.sa_flags = 0;
    if (sigaction (SIGUSR1, &disk_timer, 0) < 0) {
        perror ("Error in disk sigaction: ") ;
        exit (1) ;
    }

    /* task manager */
    task_create(&disk_manager, disk_driver_body, NULL);

    queue_remove(&ready_queue, (queue_t *) &disk_manager);
    
    disk_manager.state = SUSPENDED;

    /* inicializes the disk */
    if (disk_cmd(DISK_CMD_INIT, 0, 0) < 0)
    	return -1;
    
    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);

    if (*numBlocks < 0 || *blockSize < 0 ||
    	sem_create(&(hd.semaphore_acess), 1) == -1)
    	return -1;
    return 0;
}

int disk_block_read (int block, void *buffer) {
#ifdef DEBUG
    printf("disk_block_read: starting. block: %d buffer: %d.\n", block, *((int *) (buffer)));
#endif
	sem_down(&(hd.semaphore_acess));

	// Creates task and inserts in requisition queue
	task_request* request_t = (task_request *) malloc(sizeof(task_request));
	request_t->next = NULL;
	request_t->prev = NULL;
	request_t->request = DISK_CMD_READ;
	request_t->block = block;
	request_t->buffer = buffer;
	request_t->requester = actual;

	actual->state = SUSPENDED;
	queue_append(&disk_queue, (queue_t *) request_t);

	// if the disk manager is sleeping, wake up
	if(disk_manager.state == SUSPENDED) {
		disk_manager.state = READY;
		queue_append(&ready_queue, (queue_t *) &disk_manager);
	}

	sem_up(&(hd.semaphore_acess));
	task_switch(&dispatcher_t);

	return 0;
}

int disk_block_write (int block, void *buffer) {
#ifdef DEBUG
    printf("disk_block_write: starting. block: %d buffer: %d.\n", block, *((int *) (buffer)));
#endif
	sem_down(&(hd.semaphore_acess));

	// Creates task and inserts in requisition queue
	task_request* request_t = (task_request *) malloc(sizeof(task_request));
	request_t->next = NULL;
	request_t->prev = NULL;
	request_t->request = DISK_CMD_WRITE;
	request_t->block = block;
	request_t->buffer = buffer;
	request_t->requester = actual;

	actual->state = SUSPENDED;
	queue_append(&disk_queue, (queue_t *) request_t);

	// if the disk manager is sleeping, wake up
	if(disk_manager.state == SUSPENDED) {
		disk_manager.state = READY;
		queue_append(&ready_queue, (queue_t *) &disk_manager);
	}

	sem_up(&(hd.semaphore_acess));

	task_switch(&dispatcher_t);

	return 0;
}