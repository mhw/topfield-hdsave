#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>

/*
 * Error function specifically for Unix system call failures.
 */
extern void sys_error(char *where, char *fmt, ...);
