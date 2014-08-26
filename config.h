/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
#include "push.c"

/* appearance */
static const char font[] = "Droid Sans Mono for Powerline 9";
static const char normbordercolor[] = "#333";
static const char normbgcolor[]     = "#333";
static const char normfgcolor[]     = "#eee";
static const char selbordercolor[]  = "#17a";
static const char selbgcolor[]      = "#17a";
static const char selfgcolor[]      = "#eee";

static const unsigned int borderpx = 2;        /* border pixel of windows */
static const unsigned int snap     = 20;       /* snap pixel */
static const unsigned int systrayspacing = 2;
static const Bool showsystray      = True;
static const double shade			  = 0.8;      /* Unfocused windows */
static const Bool showbar          = True;     /* False means no bar */
static const Bool topbar           = True;     /* False means bottom bar */

/* tagging */
static const char *tags[] = {
	"2:mail", "3:www", "4:term", "q:comm", "w:doc", "d", "e", "f"
};
static const Rule rules[] = {
	/* class      instance       title    tags mask isfloating monitor focused opacity */
	{ "Gimp",     NULL,          NULL,    1 << 5,   False,     -1 },
	{ "Xmessage", NULL,          NULL,    0,        True,      -1 },
	{ NULL,       "mutt",        NULL,    1 << 0,   False,     -1 },
	{ "Surf",     NULL,          NULL,    1 << 1,   False,     -1 },
	{ NULL,       "surf",        NULL,    1 << 1,   False,     -1 },
	{ "Firefox",  NULL,          NULL,    1 << 1,   False,     -1 },
	{ NULL,       "xterm-256color", NULL,    1 << 2,   False,  -1 },
	{ NULL,       "irssi",       NULL,    1 << 3,   False,     -1 },
	{ NULL,       "Pidgin",      NULL,    1 << 3,   False,     -1 },
	{ "Evince",   NULL,          NULL,    1 << 4,   False,     -1 },
	{ "Okular",   NULL,          NULL,    1 << 4,   False,     -1 },
	{ "Ebook-viewer", NULL,      NULL,    1 << 4,   False,     -1 },
	{ "XDvi",     NULL,          NULL,    1 << 4,   False,     -1 },
	{ "MPlayer",  NULL,          NULL,    0,        True,      -1 },
	{ "Ssvnc",    NULL,	        NULL,    1 << 5,	True,	     -1 },
	{ "Toplevel", NULL,          NULL,    0,        True,      -1 },
	{ "Minitube", NULL,          NULL,    1 << 5,   False,     -1 },
	{ "XClock",   NULL,          NULL,    0,        True,      -1 },
	{ "NetHack",  "inventory",   NULL,    0,        True,      -1 },
	{ "Thunderbird", "Mail",     NULL,    1 << 7,   False,     -1 },
	{ "Thunderbird", "Calendar", NULL,    0,        True,      -1 },
	{ "Korganizer", NULL,        NULL,    1 << 7,   False,     -1 },
	{ "XConsole",  NULL,         NULL,    1 << 6,   False,     -1 },
	{ "MPlayer",  NULL,          NULL,    1 << 6,   True,      -1 },
	{ "Vlc",      NULL,          NULL,    1 << 6,   False,     -1 },
	{ "Qjackctl", NULL,          NULL,    1 << 5,   True,      -1 },
	{ NULL,       NULL,          "glxgears", 0,     True,      -1 }
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

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", font, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *termcmd[] = { "fdb", NULL };
// static const char *lockcmd[] = { "xscreensaver-command", "-lock", NULL};
static const char *lockcmd[] = { "xscreensaver-command", "-lock", NULL};
static const char *incrvolume[] = { "mixer", "vol", "+5" };
static const char *decrvolume[] = { "mixer", "vol", "-5" };

static Key keys[] = {
	/* modifier           key             function    argument */
	{ 0,                  XF86XK_AudioLowerVolume, spawn, { .v = decrvolume } },
	{ 0,                  XF86XK_AudioRaiseVolume, spawn, { .v = incrvolume } },
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
	TAGKEYS(              XK_2,                       0)
	TAGKEYS(              XK_3,                       1)
	TAGKEYS(              XK_4,                       2)
	TAGKEYS(              XK_q,                       3)
	TAGKEYS(              XK_w,                       4)
	TAGKEYS(              XK_d,                       5)
	TAGKEYS(              XK_e,                       6)
	TAGKEYS(	             XK_f,			              7)
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
