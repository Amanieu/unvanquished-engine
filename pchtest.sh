#!/bin/sh

# Test that the compiler flags are the same
mkdir -p $B
if [ "`cat $B/compiler 2> /dev/null`" != "$*" ]; then
	echo -n "$*" > $B/compiler
	echo 1
	exit 0
fi

echo 0
exit 0
