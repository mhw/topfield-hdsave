#include <stdio.h>

#include "port.h"
#include "common.h"

static char error_buffer[80];

void
error(char *where, char *what)
{
	snprintf(error_buffer, sizeof(error_buffer), "%s: %s", where, what);
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
