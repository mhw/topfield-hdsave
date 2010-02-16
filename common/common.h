/*
 * Common functions.
 */

extern uint64_t parse_disk_size(char *size);
extern char *format_disk_size(uint64_t size);

extern void error(char *where, char *what);
extern void no_memory(char *where);
extern char *get_error(void);
