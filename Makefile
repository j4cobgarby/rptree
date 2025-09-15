OBJS=history.o utils.o main.o
EXE=rptree
CFLAGS+=-g

all: $(OBJS)
	gcc -o $(EXE) $^

clean:
	rm -f $(OBJS)
	rm -f $(EXE)
