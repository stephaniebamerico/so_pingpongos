# ==================================
#||    Stephanie Briere Americo    ||
#||          GRR20165313           ||
#||     Sistemas Operacionais      ||
#||          *PingPongOS           ||
#|| Universidade Federal do Paraná ||
# ==================================

CC = gcc
CFLAGS = -Wall -g 
#-DDEBUG

BIN = bin
TEST = test
KERNEL = kernel
INCLUDE = include

all: testa_diskmanager

clean:
	-rm -f $(BIN)/task_test*
	-rm -f $(BIN)/testa*
	-rm -rf $(BIN)


#---------------------------------------------------------------

mkdir_bin:
	mkdir -p $(BIN)

#---------------------------------------------------------------

queue: $(INCLUDE)/queue.h $(KERNEL)/queue.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o queue -c $(KERNEL)/queue.c

ppos_core: $(INCLUDE)/ppos.h $(INCLUDE)/ppos_data.h $(KERNEL)/ppos_core.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o ppos_core -c $(KERNEL)/ppos_core.c

ppos_disk: $(INCLUDE)/ppos_disk.h $(KERNEL)/ppos_disk.c $(INCLUDE)/hard_disk.h $(KERNEL)/hard_disk.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o hard_disk -c $(KERNEL)/hard_disk.c -lrt
	$(CC) $(CFLAGS) -I$(INCLUDE) -o ppos_disk -c $(KERNEL)/ppos_disk.c

#---------------------------------------------------------------

testa_fila: mkdir_bin queue
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testaFila $(TEST)/testafila.c queue
	rm -f queue

testa_tasks: mkdir_bin queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/task_test1 $(TEST)/pingpong-tasks1.c queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/task_test2 $(TEST)/pingpong-tasks2.c queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/task_test3 $(TEST)/pingpong-tasks3.c queue ppos_core	
	rm -f queue ppos_core

testa_scheduler: mkdir_bin queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_scheduler $(TEST)/pingpong-scheduler.c queue ppos_core
	rm -f queue ppos_core

testa_dispatcher: mkdir_bin queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_dispatcher $(TEST)/pingpong-dispatcher.c queue ppos_core
	rm -f queue ppos_core

testa_preempcao: mkdir_bin queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_preempcao $(TEST)/pingpong-preempcao.c queue ppos_core
	rm -f queue ppos_core

testa_contab: mkdir_bin queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_contab $(TEST)/pingpong-contab.c queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_contab_prio $(TEST)/pingpong-contab-prio.c queue ppos_core
	rm -f queue ppos_core

testa_maintask: mkdir_bin queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_maintask $(TEST)/pingpong-maintask.c queue ppos_core
	rm -f queue ppos_core

testa_join: mkdir_bin queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_join $(TEST)/pingpong-join.c queue ppos_core
	rm -f queue ppos_core

testa_sleep: mkdir_bin queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_sleep $(TEST)/pingpong-sleep.c queue ppos_core
	rm -f queue ppos_core

testa_mqueue: mkdir_bin queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_mqueue $(TEST)/pingpong-mqueue.c queue ppos_core -lm
	rm -f queue ppos_core

testa_diskmanager: mkdir_bin queue ppos_core ppos_disk hard_disk $(TEST)/pingpong-disco.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_diskmanager $(TEST)/pingpong-disco.c queue ppos_core ppos_disk hard_disk -lm -lrt
	rm -f queue ppos_core ppos_disk hard_disk

