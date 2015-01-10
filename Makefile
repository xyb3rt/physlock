VERSION  := git-20150110

CC       ?= gcc
PREFIX   := /usr/local
CFLAGS   += -Wall -pedantic
CPPFLAGS += -DVERSION=\"$(VERSION)\"
LDFLAGS  +=
LIBS     := -lcrypt

.PHONY: clean install uninstall

SRC := auth.c main.c options.c util.c vt.c
OBJ := $(SRC:.c=.o)

all: physlock

physlock: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.c.o: Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

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
