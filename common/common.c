#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "port.h"
#include "common.h"

typedef struct {
	char *prefix;
	uint64_t factor;
} PrefixFactor;

PrefixFactor prefix_factors[] = {
	{ "k", 1000L },
	{ "M", 1000L*1000},
	{ "G", 1000L*1000*1000},
	{ "T", 1000L*1000*1000*1000},
};

static uint64_t
find_factor_for(const char *prefix)
{
	int i;

	for (i = 0; i < elementsof(prefix_factors); i++)
		if (strcmp(prefix, prefix_factors[i].prefix) == 0)
			return prefix_factors[i].factor;
	return 1;
}

uint64_t
parse_disk_size(char *size)
{
	char *digits_end;
	double value;

	errno = 0;
	value = strtod(size, &digits_end);
	if (errno == 0)
	{
		value = value * find_factor_for(digits_end);
		return value;
	}

	return 0;
}

static char format_buf[6];

char *
format_disk_size(uint64_t size)
{
	int i = 0;
	double value = size;
	char *out = format_buf;;

	while (value >= 1000) {
		i++;
		value = value / 1000;
	}

	out += sprintf(out, "%g", value);
	if (i > 0)
	{
		strcpy(out, prefix_factors[i-1].prefix);
	}

	return format_buf;
}

#ifdef TEST

void
main(void)
{
	printf("parse:  %" PRIu64 "\n", parse_disk_size("1.5T"));
	printf("expect: 1500000000000\n");
	printf("parse:  %" PRIu64 "\n", parse_disk_size("160G"));
	printf("expect: 160000000000\n");
	printf("format: %s\n", format_disk_size(20));
	printf("expect: 20\n");
	printf("format: %s\n", format_disk_size(1500000000000L));
	printf("expect: 1.5T\n");
	printf("format: %s\n", format_disk_size(160000000000L));
	printf("expect: 160G\n");
}

#endif
