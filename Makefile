VERSION := git-20170906

.PHONY: clean install uninstall
.SUFFIXES:

include config.mk

SRC := auth.c main.c options.c session_$(SESSION).c util.c vt.c
DEP := $(SRC:.c=.d)
OBJ := $(SRC:.c=.o)

ifeq ($(SESSION),systemd)
	LIBS += -lsystemd
endif

all: physlock

$(OBJ): Makefile

-include $(DEP)

physlock: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DVERSION=\"$(VERSION)\" -MMD -MP -c -o $@ $<

install: all
	install -D -m 4755 -o root -g root physlock $(DESTDIR)$(PREFIX)/bin/physlock
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" physlock.1 > $(DESTDIR)$(MANPREFIX)/man1/physlock.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/physlock.1

clean:
	rm -f physlock $(DEP) $(OBJ)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/physlock
	rm -f $(DESTDIR)$(MANPREFIX)/man1/physlock.1
