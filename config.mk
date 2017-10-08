PREFIX    := /usr/local
MANPREFIX := $(PREFIX)/share/man

CC        ?= gcc
CFLAGS    += -Wall -pedantic
CPPFLAGS  += -D_XOPEN_SOURCE=500
DEPFLAGS  := -MMD -MP
LDFLAGS   +=

# user detection mechanism: login/systemd/utmp
SESSION := utmp

