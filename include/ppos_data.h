// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"		// biblioteca de filas genéricas

#define STACKSIZE 32768

#define TASK_AGING -1

// task status
#define READY 0
#define RUNNING 1
#define FINISHED 2
#define SUSPENDED 3

// now, this is really confusing...
#define PRIORITY_NUM_MIN -20 // greatest priority that a task can have
#define PRIORITY_NUM_MAX 20 // lowest priority that a task can have
#define PRIORITY_DEFAULT 0

#define TICK_TIMER_US 1000
#define TICK_TIMER_S 0
#define QUANTUM 20

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t {
   struct task_t *prev, *next; // ponteiros para usar em filas
   int id; // identificador da tarefa
   int state;
   int exit_code;
   int priority_static, priority_dynamic;
   unsigned int quantum;
   unsigned int exec_time, proc_time, activations;
   void *stack; // aponta para a pilha da tarefa
   ucontext_t context ; // contexto armazenado da tarefa
   queue_t *join_tasks; // joined tasks queue
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif

