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

all: testa_tasks

clean:
	-rm $(BIN)/testaFila
	-rm $(BIN)/task_test*

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
