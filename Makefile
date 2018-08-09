VERSION = 11+

srcdir = .
VPATH = $(srcdir)

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC = cc
DEF_CFLAGS = -Wall -pedantic

# user detection mechanism: login/systemd/utmp
SESSION = utmp

ALL_CFLAGS = $(DEF_CFLAGS) $(CFLAGS)
REQ_CPPFLAGS = -I. -D_XOPEN_SOURCE=500
ALL_CPPFLAGS = $(REQ_CPPFLAGS) $(CPPFLAGS)

LIB_SESSION_login =
LIB_SESSION_systemd = -lsystemd
LIB_SESSION_utmp =
LDLIBS = -lpam -lpam_misc $(LIB_SESSION_$(SESSION))

OBJS = main.o options.o session_$(SESSION).o util.o vt.o

all: physlock

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o
$(V).SILENT:

physlock: $(OBJS)
	@echo "LINK $@"
	$(CC) $(LDFLAGS) $(ALL_CFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJS): Makefile physlock.h config.h
options.o: version.h

.c.o:
	@echo "CC $@"
	$(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS) -c -o $@ $<

config.h:
	@echo "GEN $@"
	cp $(srcdir)/config.def.h $@

version.h: Makefile .git/index
	@echo "GEN $@"
	VERSION="$$(cd $(srcdir); git describe 2>/dev/null)"; \
	[ -z "$$VERSION" ] && VERSION="$(VERSION)"; \
	echo "#define VERSION \"$$VERSION\"" >$@

.git/index:

clean:
	rm -f *.o physlock

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

