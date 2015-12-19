VERSION  := git-20151219

CC       ?= gcc
PREFIX   := /usr/local
CFLAGS   += -Wall -pedantic
CPPFLAGS += -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=500
LDFLAGS  +=
LIBS     := -lcrypt

.PHONY: clean install uninstall

SRC := auth.c main.c options.c util.c vt.c
DEP := $(SRC:.c=.d)
OBJ := $(SRC:.c=.o)

all: physlock

$(OBJ): Makefile

-include $(DEP)

physlock: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

install: all
	install -D -m 4755 -o root -g root physlock $(DESTDIR)$(PREFIX)/bin/physlock
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	sed "s/VERSION/$(VERSION)/g" physlock.1 > $(DESTDIR)$(PREFIX)/share/man/man1/physlock.1
	chmod 644 $(DESTDIR)$(PREFIX)/share/man/man1/physlock.1

clean:
	rm -f physlock $(DEP) $(OBJ)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/physlock
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/physlock.1
