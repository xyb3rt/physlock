Control physical access to a linux computer by locking all of its virtual
terminals / consoles.

physlock is an alternative to vlock, it is equivalent to `vlock -an'. It is
written because vlock blocks some linux kernel mechanisms like hibernate and
suspend and can therefore only be used with some limitations.
physlock is designed to be more lightweight, it does not have a plugin
interface and it is not started using a shell script wrapper.

Installation
------------
physlock is built using the commands:

    $ make
    # make install

Please note, that the latter one requires root privileges.
By default, physlock is installed using the prefix "/usr/local", so the full
path of the executable will be "/usr/local/sbin/physlock".

You can install it into a directory of your choice by changing the second
command to:

    # PREFIX="/your/dir" make install

Please also note, that the physlock executable will have root ownership and the
setuid bit set.

All build-time specific settings are set via preprocessor macros in the file
"config.h". Please check and change them, so that they fit your needs.

Usage
-----
The behaviour of physlock is completely controlled via command-line arguments,
it does not rely on environment variables.
It always allows unlocking as root and as a specified user. If no username is
given, physlock tries to guess the active user by locking at the file
permissions of the foreground console device (/dev/ttyX).

The following command-line arguments are supported:

    -d       fork and detach, parent returns after everything is set up
             (useful for suspend/hibernate scripts)
    -h       print short usage help and exit
    -l       only lock console switching
    -L       only enable console switching
    -v       print version information and exit
    -u USER  allow the given user to unlock the computer
