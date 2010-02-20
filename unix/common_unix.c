#include <stdio.h>
#include <stdarg.h>

#include "port.h"
#include "common.h"

static char fmt_buffer[160];
static char error_buffer[160];

void
sys_error(char *where, char *fmt, ...)
{
	va_list ap;

	snprintf(fmt_buffer, sizeof(fmt_buffer), "%s: %s: %%m", where, fmt);
	va_start(ap, fmt);
	vsnprintf(error_buffer, sizeof(error_buffer), fmt_buffer, ap);
	va_end(ap);
}

void
error(char *where, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verror(where, fmt, ap);
	va_end(ap);
}

void 
verror(char *where, char *fmt, va_list ap)
{
	snprintf(fmt_buffer, sizeof(fmt_buffer), "%s: %s", where, fmt);
	vsnprintf(error_buffer, sizeof(error_buffer), fmt_buffer, ap);
}

void
no_memory(char *where)
{
	error(where, "malloc failed");
}

char *
get_error(void)
{
	return error_buffer;
}
