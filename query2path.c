/* 

	query2path.c

	This is a store id program for the Squid proxy server
	that turns query parameters into path segments.

	Copyright 2013 by Dr. Martin Rogge

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation, version 2.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#define DEBUG 0 // note: concurrency must be 0 when debugging
#define VERBOSE 1

#define DEBUG_LOG "/var/cache/log/squid/debug.log"

#include <stdio.h>
#include <string.h>

struct transition {
	int compvalue;
	int truestate;
	int falsestate;
};

#define START	0
#define SCHEME	3
#define PATH	6
#define QUERY	7
#define EATIT	8
#define ERROR	-1

#define FALSE	0
#define TRUE	1

static struct transition machine[] = {
	// optional channel id
	{ '[',         1,     3 }, //  0
	{ ']',         2,     1 }, //  1
	{ ' ',         2,     3 }, //  2

	// scheme
	{ ':',         4,     3 }, //  3
	{ '/',         5, ERROR }, //  4
	{ '/',         6, ERROR }, //  5

	// authority and path
	{ '?',         7,     6 }, //  6

	// query and fragment
	{ ' ',         8,     7 }, //  7

	// Last entry - not used
	{ ' ',     ERROR, ERROR }
};

static int state;

#define BUFLEN	2000

static char input[BUFLEN];
static char *ip;
static char output[BUFLEN];
static char *op;

void reset() {
	ip = input;
	op = output;
	state = START;
}

int successful() {
	return state >= PATH;
}

void writeOutput(FILE *handle) {
	if (successful()) {
		fwrite("OK ", 1, 3, handle);
		fwrite(output, 1, op-output, handle);
	}
	else {
		fwrite("ERR", 1, 3, handle);
	}
	fputc('\n', handle);
}

void main (void) {

	int c;

#if DEBUG > 0
	FILE *handle = fopen(DEBUG_LOG,"a");
#endif
	reset();
	while ((c = fgetc(stdin)) != EOF) {
		if (c == '\n') {
			writeOutput(stdout);
			fflush(stdout);
#if DEBUG > 0
#if VERBOSE > 0
			fwrite(input, 1, ip-input, handle);
			fputc('\t', handle);
			writeOutput(handle);
#else
			if (successful()) {
				fwrite(input, 1, ip-input, handle);
				fputc('\n', handle);
			}
#endif
			fflush(handle);
#endif
			reset();
			continue;
		}
		if (state == ERROR) // keep parsing till end of line
			continue;
		if (ip - input >= BUFLEN - 1) { // buffer overflow
			state = ERROR;
			continue;
		}
		if (c < 32) { // unexpected control character
			state = ERROR;
			continue;
		}
		// regular character
		*ip++ = c;
		if (state == EATIT) {
			continue;
		}
		if (state >= PATH && (c == ' ' || c == '#')) { // end of URL or beginning of fragment
			state = EATIT;
			continue;
		}
		if (state == QUERY && (c == '&' || c == '='))
			c = '/';
		if (state < SCHEME || (c != '?' && c != '#' && c != '[' && c != ']')) // RFC 3986
			*op++ = c;
		else if (state == PATH && c == '?' && *(op - 1) != '/') // ugly but hey
			*op++ = '/';
		state = (c == machine[state].compvalue) ? machine[state].truestate : machine[state].falsestate;
	}
#if DEBUG > 0
	fclose(handle);
#endif
}
