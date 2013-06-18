#!/bin/sh

# Test for strlcpy and strlcat
echo "\
#include <string.h>

int main(void)
{
	char *str1, *str2;
	strlcpy(str1, str2, 1);
	strlcat(str1, str2, 1);
	return 0;
}
" | $* -xc++ - -o /dev/null 2> /dev/null && echo "#define HAVE_STRLCPY"

exit 0
