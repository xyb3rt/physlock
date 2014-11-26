all: physlock

VERSION = git-20141126

CC      = gcc
PREFIX  = /usr/local
CFLAGS  = -Wall -pedantic -DVERSION=\"$(VERSION)\"
LDFLAGS =
LIBS    = -lcrypt

SRC = auth.c main.c options.c util.c vt.c
OBJ = $(SRC:.c=.o)

physlock:	$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.c.o: Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

install: all
	install -D -m 4755 -o root -g root physlock $(DESTDIR)$(PREFIX)/bin/physlock
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	sed "s/VERSION/$(VERSION)/g" physlock.1 > $(DESTDIR)$(PREFIX)/share/man/man1/physlock.1
	chmod 644 $(DESTDIR)$(PREFIX)/share/man/man1/physlock.1

clean:
	rm -f physlock $(OBJ)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/physlock
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/physlock.1
