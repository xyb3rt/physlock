Control physical access to a linux computer by locking all of its virtual
terminals / consoles.

physlock is an alternative to vlock, it is equivalent to `vlock -an'. It is
written because vlock blocks some linux kernel mechanisms like hibernate and
suspend and can therefore only be used with some limitations.
physlock is designed to be more lightweight, it does not have a plugin
interface and it is not started using a shell script wrapper.

physlock tries to detect the user logged in on the active console by first
searching the utmp file for an entry whose `ut_line` field equals the device
name of the active console, e.g. "tty1". If no such entry is found, then
physlock falls back to the owner of the device file of the active console. Some
graphical login managers do neither write an appropriate utmp entry nor set the
owner of the device file. You have to manually set up `sessreg(1)` in order to
use physlock with such a graphical login manager.

Installation
------------
physlock is built using the commands:

    $ make
    # make install

Please note, that the latter one requires root privileges.
By default, physlock is installed using the prefix "/usr/local", so the full
path of the executable will be "/usr/local/bin/physlock".

You can install it into a directory of your choice by changing the second
command to:

    # make PREFIX="/your/dir" install

Please also note, that the physlock executable will have root ownership and the
setuid bit set.

All build-time specific settings are set via preprocessor macros in the file
"config.h". Please check and change them, so that they fit your needs.

Usage
-----
The behaviour of physlock is completely controlled via command-line arguments,
it does not rely on environment variables.
physlock uses the utmp file to identify the owner of the current session (i.e.
active tty) and prompts for her password to unlock the computer.

The following command-line arguments are supported:

    -d       fork and detach, parent returns after everything is set up
             (useful for suspend/hibernate scripts)
    -h       print short usage help and exit
    -l       only lock console switching
    -L       only enable console switching
    -m       mute kernel messages on console while physlock is running
    -s       disable sysrq key while physlock is running
    -p MSG   Display MSG before the password prompt
    -v       print version information and exit
