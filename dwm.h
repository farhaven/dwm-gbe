#ifndef _DWM_H
#define _DWM_H
#include <X11/keysym.h>

typedef struct Monitor Monitor;
typedef struct Client Client;

struct Monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	Bool topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Window barwin;
};

struct Client {
	char *class, *instance;
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	unsigned int tags;
	Bool isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	Client *next;
	Client *snext;
	Monitor *mon;
	Window win;
};

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

void grabkey(int, KeySym, int);

Monitor *dirtomon(int dir);
void focusmon(const Arg *arg);
void focusstack(int);
void killclient(Client *);
void sendmon(Client *c, Monitor *m);
void tag(Client *, unsigned int);
void togglefloating(Client *);
void toggletag(Client *, unsigned int);
void toggleview(unsigned int);
void view(unsigned int);
void zoom(Client *);

unsigned int getsystraywidth();
#endif /* _DWM_H */
