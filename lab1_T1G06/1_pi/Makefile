CC=gcc
CFLAGS=-fopenmp

OBJ=o
EXE=.c
RM=rm

EXES=pi_seq pi_par pi_task

all: $(EXES)

seq: pi_seq.c
	$(CC) pi_seq.c -o pi_seq $(CFLAGS)

par: pi_par.c
	$(CC) pi_par.c -o pi_par $(CFLAGS)

task: pi_task.c
	$(CC) pi_task.c -o pi_task $(CFLAGS)

clean:
	$(RM) $(EXES) *.$(OBJ)

