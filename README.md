Control physical access to a linux computer by locking all of its virtual
terminals.

physlock only allows the user of the active session (the user logged in on the
foreground virtual terminal) and the root user to unlock the computer and uses
PAM for authentication.

physlock supports 3 mechanisms to detect the user of the active session:

1. *login*: Using the owner of the corresponding tty device file typically set
   by login(1)
2. *utmp*: Searching the utmp file for an entry whose `ut_line` field equals
   the tty device name
3. *systemd*: Querying systemd-logind(1)

The used mechanism is selected at build time by setting the `SESSION` macro.
The default is `utmp`.

Installation
------------
physlock is build using the commands:

    $ make
    # make install

Please note, that the latter one requires root privileges.
By default, physlock is installed using the prefix `/usr/local`, so the full
path of the executable will be `/usr/local/bin/physlock`.

You can install it into a directory of your choice by overwriting the `PREFIX`
macro in the second command:

    # make PREFIX="/your/dir" install

Please also note, that the physlock executable will have root ownership and the
setuid bit set.

All build-time specific settings are set in the file `config.h`. Please check
and change them, so that they fit your needs. If the file `config.h` does not
already exist, then you have to create it with the following command:

    $ make config.h

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
    -p MSG   Display MSG before the password prompt
    -s       disable sysrq key while physlock is running
    -v       print version information and exit
