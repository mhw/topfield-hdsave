/*
 * Implementation of common utility routines on Unix platforms.
 *
 * Copyright 2010 Mark H. Wilkinson
 *
 * This file is part of HDSave.
 *
 * HDSave is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HDSave is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HDSave.  If not, see <http://www.gnu.org/licenses/>.
 */

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

void
vwarn(char *fmt, va_list ap)
{
	fputs("warning: ", stderr);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
}

void
fatal(char *where, char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "FATAL: %s: ", where);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputs("\n", stderr);
}
