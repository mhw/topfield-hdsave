/*
 * Common functions.
 */

#define elementsof(array) (sizeof(array)/sizeof(array[0]))

extern uint64_t parse_disk_size(char *size);
extern char *format_disk_size(uint64_t size);

extern void error(char *where, char *fmt, ...);
extern void verror(char *where, char *fmt, va_list ap);
extern void no_memory(char *where);
extern char *get_error(void);

extern void vwarn(char *fmt, va_list ap);
