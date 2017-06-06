#include <X11/keysym.h>

#include "dwm.h"

int l_call_client_click(int, int, Client *);
int l_call_client_new(Client *);
int l_call_keypress(unsigned int, KeySym);
int l_call_status_click(int, int);
int l_call_status_drawfn(int, int, int);
int l_call_tag_click(int, int, int);

void l_init();
void l_loadconfig();
