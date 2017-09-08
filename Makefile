VERSION := git-20170908

.PHONY: all clean install uninstall
.SUFFIXES:

include config.mk

VPATH := $(SRCDIR)

CPPFLAGS += -I. -DVERSION=\"$(VERSION)\"
DEPFLAGS := -MMD -MP

LDLIBS := -lpam -lpam_misc

ifeq ($(SESSION),systemd)
	LDLIBS += -lsystemd
endif

SRC := auth.c main.c options.c session_$(SESSION).c util.c vt.c
DEP := $(SRC:.c=.d)
OBJ := $(SRC:.c=.o)

all: physlock

$(OBJ): Makefile

-include $(DEP)

physlock: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c -o $@ $<

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
