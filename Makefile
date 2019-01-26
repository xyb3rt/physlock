version = 12

srcdir = .
VPATH = $(srcdir)

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

# user detection mechanism: login/systemd/utmp
SESSION = utmp

cflags = -Wall -pedantic $(CFLAGS)
cppflags = -I. $(CPPFLAGS) -D_XOPEN_SOURCE=500

lib_session_login =
lib_session_systemd = -lsystemd
lib_session_utmp =
ldlibs = $(LDLIBS) -lpam -lpam_misc $(lib_session_$(SESSION))

objs = main.o options.o session_$(SESSION).o util.o vt.o

all: physlock

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o
$(V).SILENT:

physlock: $(objs)
	@echo "LINK $@"
	$(CC) $(LDFLAGS) -o $@ $(objs) $(ldlibs)

$(objs): Makefile physlock.h config.h
options.o: version.h

.c.o:
	@echo "CC $@"
	$(CC) $(cflags) $(cppflags) -c -o $@ $<

config.h:
	@echo "GEN $@"
	cp $(srcdir)/config.def.h $@

version.h: Makefile .git/index
	@echo "GEN $@"
	v="$$(cd $(srcdir); git describe 2>/dev/null)"; \
	echo "#define VERSION \"$${v:-$(version)}\"" >$@

.git/index:

clean:
	rm -f *.o physlock

install: all
	@echo "INSTALL bin/physlock"
	install -D -m 4755 -o root -g root physlock $(DESTDIR)$(PREFIX)/bin/physlock
	@echo "INSTALL physlock.1"
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(version)/g" physlock.1 > $(DESTDIR)$(MANPREFIX)/man1/physlock.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/physlock.1

uninstall:
	@echo "REMOVE bin/physlock"
	rm -f $(DESTDIR)$(PREFIX)/bin/physlock
	@echo "REMOVE physlock.1"
	rm -f $(DESTDIR)$(MANPREFIX)/man1/physlock.1

