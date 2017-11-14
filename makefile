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

all: testa_mqueue

clean:
	-rm $(BIN)/task_test*
	-rm $(BIN)/testa*

#---------------------------------------------------------------

queue: $(INCLUDE)/queue.h $(KERNEL)/queue.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o queue -c $(KERNEL)/queue.c

ppos_core: $(INCLUDE)/ppos.h $(INCLUDE)/ppos_data.h $(KERNEL)/ppos_core.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o ppos_core -c $(KERNEL)/ppos_core.c

#---------------------------------------------------------------

testa_fila: queue
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testaFila $(TEST)/testafila.c queue
	rm -f queue

testa_tasks: queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/task_test1 $(TEST)/pingpong-tasks1.c queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/task_test2 $(TEST)/pingpong-tasks2.c queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/task_test3 $(TEST)/pingpong-tasks3.c queue ppos_core	
	rm -f queue ppos_core

testa_scheduler: queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_scheduler $(TEST)/pingpong-scheduler.c queue ppos_core
	rm -f queue ppos_core

testa_dispatcher: queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_dispatcher $(TEST)/pingpong-dispatcher.c queue ppos_core
	rm -f queue ppos_core

testa_preempcao: queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_preempcao $(TEST)/pingpong-preempcao.c queue ppos_core
	rm -f queue ppos_core

testa_contab: queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_contab $(TEST)/pingpong-contab.c queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_contab_prio $(TEST)/pingpong-contab-prio.c queue ppos_core
	rm -f queue ppos_core

testa_maintask: queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_maintask $(TEST)/pingpong-maintask.c queue ppos_core
	rm -f queue ppos_core

testa_join: queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_join $(TEST)/pingpong-join.c queue ppos_core
	rm -f queue ppos_core

testa_sleep: queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_sleep $(TEST)/pingpong-sleep.c queue ppos_core
	rm -f queue ppos_core

testa_mqueue: queue ppos_core
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testa_mqueue $(TEST)/pingpong-mqueue.c queue ppos_core -lm
	rm -f queue ppos_core