VERSION := git-20170908

all: physlock

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

$(OBJ): Makefile config.h

physlock: $(OBJ)
	@echo "LINK $@"
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	@echo "CC $<"
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c -o $@ $<

config.h:
	@echo "GEN $@"
	cp $(SRCDIR)/config.def.h $@

clean:
	rm -f physlock $(DEP) $(OBJ)

install: all
	@echo "INSTALL bin/physlock"
	install -D -m 4755 -o root -g root physlock $(DESTDIR)$(PREFIX)/bin/physlock
	@echo "INSTALL physlock.1"
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" physlock.1 > $(DESTDIR)$(MANPREFIX)/man1/physlock.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/physlock.1

uninstall:
	@echo "REMOVE bin/physlock"
	rm -f $(DESTDIR)$(PREFIX)/bin/physlock
	@echo "REMOVE physlock.1"
	rm -f $(DESTDIR)$(MANPREFIX)/man1/physlock.1

.PHONY: all clean install uninstall
.SUFFIXES:
$(V).SILENT:

-include $(DEP)

