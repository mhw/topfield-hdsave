/*
 * Some general purpose utility functions.
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

#define elementsof(array) (sizeof(array)/sizeof(array[0]))

extern uint64_t parse_disk_size(char *size);
extern char *format_disk_size(uint64_t size);

extern void error(char *where, char *fmt, ...);
extern void verror(char *where, char *fmt, va_list ap);
extern void no_memory(char *where);
extern char *get_error(void);

extern void vwarn(char *fmt, va_list ap);

extern void fatal(char *where, char *fmt, ...);
