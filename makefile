# ==================================
#||    Stephanie Briere Americo    ||
#||          GRR20165313           ||
#||     Sistemas Operacionais      ||
#||          *PingPongOS           ||
#|| Universidade Federal do Paraná ||
# ==================================

CC = gcc
CFLAGS = -Wall -g

BIN = bin
TEST = test
KERNEL = kernel
INCLUDE = include

all: queue testaFila

clean:
	-rm $(BIN)/testaFila

#---------------------------------------------------------------

queue: $(INCLUDE)/queue.h $(KERNEL)/queue.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o queue -c $(KERNEL)/queue.c

testaFila: queue
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $(BIN)/testaFila $(TEST)/testafila.c queue
	rm -f $(BIN)/queue
