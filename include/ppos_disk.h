// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.2 -- Julho de 2017

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

#include "ppos_data.h" // semaphore

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

// estrutura que representa um disco no sistema operacional
typedef struct {
  int signal;
  semaphore_t semaphore_acess;
} disk_t;

// estrutura que especifica a requisição feita ao disco 
typedef struct {
  struct task_request *next, *prev;
  int request, block;
  void *buffer;
  task_t *requester;
} task_request;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize);

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer);

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer);

#endif
