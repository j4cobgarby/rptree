OBJS=history.o utils.o rptree.o
EXE=rptree
CC=gcc
LD=gcc

CFLAGS+=-Wall -Wextra
CFLAGS+=-g

$(EXE): $(OBJS)

clean:
	rm -f $(OBJS) $(EXE)
