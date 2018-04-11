VERSION = git-20180411

srcdir = .
VPATH = $(srcdir)

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC = cc
DEF_CFLAGS = -Wall -pedantic

# user detection mechanism: login/systemd/utmp
SESSION = utmp

ALL_CFLAGS = $(DEF_CFLAGS) $(CFLAGS)
REQ_CPPFLAGS = -I. -D_XOPEN_SOURCE=500 -DVERSION=\"$(VERSION)\"
ALL_CPPFLAGS = $(REQ_CPPFLAGS) $(CPPFLAGS)

LIB_SESSION_login =
LIB_SESSION_systemd = -lsystemd
LIB_SESSION_utmp =
LDLIBS = -lpam -lpam_misc $(LIB_SESSION_$(SESSION))

OBJS = auth.o main.o options.o session_$(SESSION).o util.o vt.o

all: physlock

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o
$(V).SILENT:

physlock: $(OBJS)
	@echo "LINK $@"
	$(CC) $(LDFLAGS) $(ALL_CFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJS): Makefile config.h

.c.o:
	@echo "CC $@"
	$(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS) -MMD -MP -c -o $@ $<

config.h:
	@echo "GEN $@"
	cp $(srcdir)/config.def.h $@

clean:
	rm -f *.o *.d physlock

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

-include $(OBJS:.o=.d)

