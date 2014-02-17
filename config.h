/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
#include "push.c"

/* appearance */
static const char font[] = "Droid Sans Mono 10";
static const char normbordercolor[] = "#444";
static const char normbgcolor[]     = "#bbb";
static const char normfgcolor[]     = "#222";
static const char selbordercolor[]  = "#057";
static const char selbgcolor[]      = "#057";
static const char selfgcolor[]      = "#eee";

static const unsigned int borderpx = 5;        /* border pixel of windows */
static const unsigned int snap     = 32;       /* snap pixel */
static const unsigned int systrayspacing = 2;
static const Bool showsystray      = True;
static const double shade			  = 0.8;      /* Unfocused windows */
static const Bool showbar          = True;     /* False means no bar */
static const Bool topbar           = True;     /* False means bottom bar */

/* tagging */
static const char *tags[] = {
	"1:", "2:", "3:", "4:⌨", "q:", "w:", "d:", "e:", "f:"
};
static const Rule rules[] = {
	/* class      instance       title    tags mask isfloating monitor opacity */
	{ "Gimp",     NULL,          NULL,    1 << 6,   False,     -1,     -1 },
	{ "Xmessage", NULL,          NULL,    0,        True,      -1,     -1 },
	{ NULL,       "mutt",        NULL,    1 << 1,   False,     -1,     -1 },
	{ "Surf",     NULL,          NULL,    1 << 2,   False,     -1,     -1 },
	{ NULL,       "surf",        NULL,    1 << 2,   False,     -1,     -1 },
	{ NULL,       "st-256color", NULL,    1 << 3,   False,     -1,     -1 },
	{ NULL,       "irssi",       NULL,    1 << 4,   False,     -1,     -1 },
	{ NULL,       "Pidgin",      NULL,    1 << 4,   False,     -1,     -1 },
	{ "Evince",   NULL,          NULL,    1 << 5,   False,     -1,     -1 },
	{ "Okular",   NULL,          NULL,    1 << 5,   False,     -1,     -1 },
	{ "Ebook-viewer", NULL,      NULL,    1 << 5,   False,     -1,     -1 },
	{ "XDvi",     NULL,          NULL,    1 << 5,   False,     -1,     -1 },
	{ "MPlayer",  NULL,          NULL,    0,        True,      -1,     -1 },
	{ "Gvim",     NULL,          NULL,    1 << 0,  	False,     -1,     -1 },
	{ "Ssvnc",    NULL,	        NULL,    1 << 6,	True,	     -1,     -1 },
	{ "Toplevel", NULL,          NULL,    0,        True,      -1,     -1 },
	{ "Minitube", NULL,          NULL,    1 << 6,   False,     -1,     -1 },
	{ "XClock",   NULL,          NULL,    0,        True,      -1,     -1 },
	{ "NetHack",  "inventory",   NULL,    0,        True,      -1,     -1 },
	{ "Thunderbird", "Mail",     NULL,    1 << 8,   False,     -1,     -1 },
	{ "Thunderbird", "Calendar", NULL,    0,        True,      -1,     -1 },
	{ "XConsole",  NULL,         NULL,    1 << 7,   False,     -1,     -1 },
	{ "MPlayer",  NULL,          NULL,    1 << 7,   True,      -1,     -1 }
};

/* layout(s) */
static const float mfact      = 1.0 / 1.618; /* reverse of the golden cut */
static const int nmaster      = 1;    /* number of clients in master area */
static const Bool resizehints = True; /* True means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "[M]",      monocle },
	{ "><>",      NULL },    /* no layout function means floating behavior */
};

/* key definitions */
#define MODKEY Mod3Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      toggleview, {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      view,       {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,        {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,  {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", font, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *termcmd[] = { "fdb", NULL };
static const char *lockcmd[] = { "xscreensaver-command", "-lock", NULL };

static Key keys[] = {
	/* modifier           key             function    argument */
	{ MODKEY,             XK_Return,      spawn,      {.v = termcmd } },
	{ MODKEY,             XK_l,           spawn,      {.v = lockcmd } },
	{ MODKEY,             XK_b,           togglebar,  {0} },
	{ MODKEY,             XK_Down, 	     focusstack, {.i = +1 } },
	{ MODKEY,             XK_Up,   	     focusstack, {.i = -1 } },
	{ MODKEY | Mod1Mask,  XK_Right,	     setmfact,   {.f = +0.05} },
	{ MODKEY | Mod1Mask,  XK_Left, 	     setmfact,   {.f = -0.05} },
	{ MODKEY,             XK_Left, 	     zoom,       {0} },
	{ MODKEY,	          XK_Right,       pushdown,   {0} },
	{ MODKEY,             XK_r,    	     view,       {0} },
	{ MODKEY | ShiftMask, XK_c,           killclient, {0} },
	{ MODKEY,             XK_0,           toggleview, {.ui = ~0 } },
	{ MODKEY | ShiftMask, XK_0,	        toggletag,  {.ui = ~0 } },
	{ 0,                  XF86XK_Back,    focusmon,   {.i = -1 } },
	{ 0,                  XF86XK_Forward, focusmon,   {.i = +1 } },
	{ MODKEY, 	          XF86XK_Back,    tagmon,     {.i = -1 } },
	{ MODKEY,   	       XF86XK_Forward, tagmon,     {.i = +1 } },
	TAGKEYS(              XK_1,                       0)
	TAGKEYS(              XK_2,                       1)
	TAGKEYS(              XK_3,                       2)
	TAGKEYS(              XK_4,                       3)
	TAGKEYS(              XK_q,                       4)
	TAGKEYS(              XK_w,                       5)
	TAGKEYS(              XK_d,                       6)
	TAGKEYS(              XK_e,                       7)
	TAGKEYS(	             XK_f,			              8)
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click         event mask button   function        argument */
	{ ClkLtSymbol,   0,         Button1, setlayout,      {.v = &layouts[0]} },
	{ ClkLtSymbol,	  0,		    Button2, setlayout,      {.v = &layouts[1]} },
	{ ClkLtSymbol,   0,         Button3, setlayout,      {.v = &layouts[2]} },

	{ ClkWinTitle,   0,         Button1, focusstack,     {.i = -1} },
	{ ClkWinTitle,	  0,		    Button2, zoom,	        { 0 } },
	{ ClkWinTitle,	  0,	       Button3, focusstack,     {.i = 1 } },
	{ ClkStatusText, 0,		    Button2, killclient,	  {0} },
	{ ClkStatusText, 0,         Button1, spawn,          {.v = termcmd } },

	{ ClkClientWin,  MODKEY,    Button1, movemouse,      {0} },
	{ ClkClientWin,  MODKEY,    Button2, togglefloating, {0} },
	{ ClkClientWin,  MODKEY,    Button3, resizemouse,    {0} },

	{ ClkTagBar,     0,         Button1, toggleview,     {0} },
	{ ClkTagBar,     0,         Button3, view,           {0} },
	{ ClkTagBar,     MODKEY,    Button1, toggletag,      {0} },
	{ ClkTagBar,     MODKEY,    Button3, tag,            {0} },
};
