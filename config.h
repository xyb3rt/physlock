/* pseudo device that points to foreground console:    */
/* (typically "/dev/console" or "/dev/tty0")           */
static const char * const CONSOLE_DEVICE  = "/dev/console";

/* common basename of all virtual console devices:     */
static const char * const TTY_DEVICE_BASE = "/dev/tty";

/* timeout (seconds) after failed authentication try:  */
enum { AUTH_FAIL_TIMEOUT = 2 };

