/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

/* appearance */
// static const char font[] = "Droid Sans Mono for Powerline 7.5";
// static const char font[] = "Terminess Powerline 9";
static const char font[] = "PragmataPro for Powerline:size=9";
static const char normbordercolor[] = "#ccc";
static const char normbgcolor[]     = "#ccc";
static const char normfgcolor[]     = "#000";
static const char selbordercolor[]  = "#666";
static const char selbgcolor[]      = "#666";
static const char selfgcolor[]      = "#fff";

static const unsigned int borderpx = 2;        /* border pixel of windows */
static const unsigned int snap     = 20;       /* snap pixel */
static const unsigned int systrayspacing = 2;
static const Bool showsystray      = True;
static const Bool topbar           = True;     /* False means bottom bar */

/* tagging */
static const char *tags[] = {
	"2:mail", "3:www", "4:term", "q:comm", "w:doc", "d", "e", "f:cal", "junk"
};

/* layout */
static const float mfact      = 0.58;
static const int nmaster      = 1;
static const Bool resizehints = False;

/* key definitions */
#define MODKEY Mod4Mask

static Key keys[] = {
	/* modifier           key             function    argument */
	{ MODKEY | ShiftMask, XK_l,           l_loadconfig, {0} },
	{ MODKEY,             XK_Down,        focusstack, {.i = +1 } },
	{ MODKEY,             XK_Up,          focusstack, {.i = -1 } },
	{ MODKEY | Mod1Mask,  XK_Right,       setmfact,   {.f = +0.01} },
	{ MODKEY | Mod1Mask,  XK_Left,        setmfact,   {.f = -0.01} },
	{ MODKEY,             XK_Left,        zoom,       {0} },
	{ MODKEY,             XK_Right,       pushdown,   {0} },
	{ MODKEY | ShiftMask, XK_c,           killclient, {0} },
	{ MODKEY,             XK_space,       focusmon,   {.i = -1 } },
	{ MODKEY | ShiftMask, XK_space,       tagmon,     {.i = -1 } },
	{ 0,                  XF86XK_Back,    focusmon,   {.i = -1 } },
	{ MODKEY,             XF86XK_Back,    tagmon,     {.i = -1 } },
};

/* button definitions */
/* click can be ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click         event mask button   function        argument */
	{ ClkWinTitle,   0,         Button1, focusstack,     {.i = -1} },
	{ ClkWinTitle,   0,         Button2, zoom,           {0} },
	{ ClkWinTitle,   0,         Button3, focusstack,     {.i = 1 } },

	{ ClkClientWin,  MODKEY,    Button1, movemouse,      {0} },
	{ ClkClientWin,  MODKEY,    Button2, togglefloating, {0} },
	{ ClkClientWin,  MODKEY,    Button3, resizemouse,    {0} },
};
