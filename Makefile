all: physlock

CC?=gcc
PREFIX?=/usr/local
CFLAGS+= -std=c99 -Wall -pedantic -g
LDFLAGS+= 
LIBS+= -lcrypt

SRCFILES=$(wildcard *.c)
OBJFILES=$(SRCFILES:.c=.o)

physlock:	$(OBJFILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

install: all
	install -D -m 4755 -o root -g root physlock $(PREFIX)/sbin/physlock

clean:
	rm -f physlock *.o

tags: *.h *.c
	ctags $^

cscope: *.h *.c
	cscope -b
