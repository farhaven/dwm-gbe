/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

/* appearance */
// static const char font[] = "Droid Sans Mono for Powerline 7.5";
// static const char font[] = "Terminess Powerline 9";
static const char font[] = "PragmataPro for Powerline 8";
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
static const Bool showbar          = True;     /* False means no bar */
static const Bool topbar           = True;     /* False means bottom bar */

/* tagging */
static const char *tags[] = {
	"2:mail", "3:www", "4:term", "q:comm", "w:doc", "d", "e", "f:cal"
};
enum {
	T_NONE  = 0,
	T_MAIL  = 1 << 0,
	T_WWW   = 1 << 1,
	T_TERM  = 1 << 2,
	T_COMM  = 1 << 3,
	T_DOC   = 1 << 4,
	T_MISC1 = 1 << 5,
	T_MISC2 = 1 << 6,
	T_CAL   = 1 << 7
};
static const Rule rules[] = {
	/* class      instance       title    tags mask isfloating monitor */
	{ "Gimp",     NULL,          NULL,    T_MISC1,  False,     -1 },
	{ "Xmessage", NULL,          NULL,    T_NONE,   True,      -1 },
	{ NULL,       "mutt",        NULL,    T_MAIL,   False,     -1 },
	{ "Surf",     NULL,          NULL,    T_WWW,    False,     -1 },
	{ NULL,       "surf",        NULL,    T_WWW,    False,     -1 },
	{ "Firefox",  NULL,          NULL,    T_WWW,    False,     -1 },
	{ NULL,       "xterm-256color", NULL, T_TERM,   False,     -1 },
	{ NULL,       "st-256color", NULL,    T_TERM,   False,     -1 },
	{ NULL,       "irssi",       NULL,    T_COMM,   False,     -1 },
	{ NULL,       "Pidgin",      NULL,    T_COMM,   False,     -1 },
	{ "Evince",   NULL,          NULL,    T_DOC,    False,     -1 },
	{ "Xpdf",     NULL,          NULL,    T_DOC,    False,     -1 },
	{ "Okular",   NULL,          NULL,    T_DOC,    False,     -1 },
	{ "Ebook-viewer", NULL,      NULL,    T_DOC,    False,     -1 },
	{ "XDvi",     NULL,          NULL,    T_DOC,    False,     -1 },
	{ "Ssvnc",    NULL,          NULL,    T_MISC1,  True,      -1 },
	{ "Toplevel", NULL,          NULL,    T_NONE,   True,      -1 },
	{ "Minitube", NULL,          NULL,    T_MISC1,  False,     -1 },
	{ "Qjackctl", NULL,          NULL,    T_MISC1,  True,      -1 },
	{ "libreoffice", NULL,       NULL,    T_DOC,    False,     -1 },
	{ NULL,       "transmission-gtk", NULL,T_MISC1, False,     -1 },
	{ NULL,       NULL,          "LibreOffice", T_DOC, False,  -1 },
	{ "XClock",   NULL,          NULL,    T_NONE,   True,      -1 },
	{ "NetHack",  "inventory",   NULL,    T_NONE,   True,      -1 },
	{ "Tkremind", NULL,          NULL,    T_CAL,    False,     -1 },
	{ "XConsole",  NULL,         NULL,    T_MISC2,  False,     -1 },
	{ "MPlayer",  NULL,          NULL,    T_MISC2,  True,      -1 },
	{ "Vlc",      NULL,          NULL,    T_MISC2,  False,     -1 },
	{ "Inkscape", NULL,          NULL,    T_MISC2,  False,     -1 },
	{ NULL,       NULL,          "glxgears", T_NONE,True,      -1 },
	{ NULL,       NULL,          "livestream", T_MISC2, True,  -1 }
};

/* layout(s) */
static const float mfact      = 0.58;
static const int nmaster      = 1;
static const Bool resizehints = False;

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
/* component of dmenucmd, manipulated in spawn() */
static char dmenumon[2] = "0";
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", font,
	"-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor,
	NULL };
static const char *termcmd[] = { "fdb", NULL };
static const char *lockcmd[] = { "xscreensaver-command", "-lock", NULL};

#define MPD_CMD(___c) { \
	.v = (char *[]) { \
		"mpc", ___c, NULL \
	} \
}

static Key keys[] = {
	/* modifier           key             function    argument */
	{ MODKEY,             XK_Return,      spawn,      {.v = termcmd } },
	{ MODKEY,             XK_l,           spawn,      {.v = lockcmd } },
	{ MODKEY,             XK_b,           togglebar,  {0} },
	{ MODKEY,             XK_Down,        focusstack, {.i = +1 } },
	{ MODKEY,             XK_Up,          focusstack, {.i = -1 } },
	{ MODKEY | Mod1Mask,  XK_Right,       setmfact,   {.f = +0.01} },
	{ MODKEY | Mod1Mask,  XK_Left,        setmfact,   {.f = -0.01} },
	{ MODKEY,             XK_Left,        zoom,       {0} },
	{ MODKEY,             XK_Right,       pushdown,   {0} },
	{ MODKEY,             XK_r,           view,       {0} },
	{ MODKEY | ShiftMask, XK_c,           killclient, {0} },
	{ MODKEY,             XK_0,           toggleview, {.ui = ~0 } },
	{ MODKEY | ShiftMask, XK_0,           toggletag,  {.ui = ~0 } },
	{ MODKEY,             XK_space,   focusmon,   {.i = -1 } },
	{ MODKEY | ShiftMask, XK_space,   tagmon,     {.i = -1 } },
	{ 0,                  XF86XK_Back,    focusmon,   {.i = -1 } },
	{ MODKEY,             XF86XK_Back,    tagmon,     {.i = -1 } },
	{ 0,                  XF86XK_AudioStop, spawn,    MPD_CMD("stop") },
	{ 0,                  XF86XK_AudioPlay, spawn,    MPD_CMD("toggle") },
	{ 0,                  XF86XK_AudioPrev, spawn,    MPD_CMD("prev") },
	{ 0,                  XF86XK_AudioNext, spawn,    MPD_CMD("next") },
	TAGKEYS(              XK_2,                       0)
	TAGKEYS(              XK_3,                       1)
	TAGKEYS(              XK_4,                       2)
	TAGKEYS(              XK_q,                       3)
	TAGKEYS(              XK_w,                       4)
	TAGKEYS(              XK_d,                       5)
	TAGKEYS(              XK_e,                       6)
	TAGKEYS(              XK_f,                       7)
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin,
 * or ClkRootWin */
static Button buttons[] = {
	/* click         event mask button   function        argument */
	{ ClkLtSymbol,   0,         Button1, setlayout,      {.v = &layouts[0]} },
	{ ClkLtSymbol,   0,         Button2, setlayout,      {.v = &layouts[1]} },
	{ ClkLtSymbol,   0,         Button3, setlayout,      {.v = &layouts[2]} },

	{ ClkWinTitle,   0,         Button1, focusstack,     {.i = -1} },
	{ ClkWinTitle,   0,         Button2, zoom,           {0} },
	{ ClkWinTitle,   0,         Button3, focusstack,     {.i = 1 } },
	{ ClkStatusText, 0,         Button2, killclient,     {0} },
	{ ClkStatusText, 0,         Button1, spawn,          {.v = termcmd } },

	{ ClkClientWin,  MODKEY,    Button1, movemouse,      {0} },
	{ ClkClientWin,  MODKEY,    Button2, togglefloating, {0} },
	{ ClkClientWin,  MODKEY,    Button3, resizemouse,    {0} },

	{ ClkTagBar,     0,         Button1, toggleview,     {0} },
	{ ClkTagBar,     0,         Button3, view,           {0} },
	{ ClkTagBar,     MODKEY,    Button1, toggletag,      {0} },
	{ ClkTagBar,     MODKEY,    Button3, tag,            {0} },
};
