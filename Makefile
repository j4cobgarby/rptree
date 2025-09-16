OBJS_RPTREE=history.o utils.o rptree.o
OBJS_TRACEME=traceme.o

RPTREE=rptree
TRACEME=traceme

CC=gcc
LD=gcc

CFLAGS+=-Wall -Wextra
CFLAGS+=-g

all: $(RPTREE) $(TRACEME)

$(RPTREE): $(OBJS_RPTREE)

$(TRACEME): $(OBJS_TRACEME)

clean:
	rm -f $(OBJS_RPTREE) $(OBJS_TRACEME) $(RPTREE) $(TRACEME)
