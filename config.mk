PREFIX    := /usr/local
MANPREFIX := $(PREFIX)/share/man

CC        ?= gcc
CFLAGS    += -Wall -pedantic
CPPFLAGS  += -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=500
LDFLAGS   +=
LIBS      := -lpam -lpam_misc

# user detection mechanism: login/systemd/utmp
SESSION := utmp

