/* See LICENSE file for copyright and license details. */

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

void die(const char *errstr, ...);

/* Functions from dwm.c */
void spawn(const Arg *arg);
unsigned int getsystraywidth();

enum { SchemeNorm, SchemeSel, SchemeLast }; /* color schemes */
