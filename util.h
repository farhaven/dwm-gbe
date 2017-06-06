/* See LICENSE file for copyright and license details. */
#include "dwm.h"

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))

/* Functions from dwm.c */
void spawn(const Arg *arg);
unsigned int getsystraywidth();

enum { SchemeNorm, SchemeSel, SchemeLast }; /* color schemes */
