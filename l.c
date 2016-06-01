#include <assert.h>
#include <err.h>
#include <stdlib.h>
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <X11/keysym.h>

#include "drw.h"
#include "l.h"
#include "dwm.h"

#define typeassert(L, index, type) \
	if (!lua_is##type(L, index)) { \
		return luaL_error(L, "Expected a " #type ", got a %s", \
		                  lua_typename(L, lua_type(L, index))); \
	}

lua_State *globalL = NULL;
ClrScheme *l_scheme = NULL;

extern char stext[256];
extern int bh;
extern Drw *drw;
extern Monitor *selmon;

static int l_u_client_current(lua_State*);
static int l_u_client_focusstack(lua_State*);
static int l_u_client_index(lua_State*);
static int l_u_drw_setscheme(lua_State*);
static int l_u_drw_text(lua_State*);
static int l_u_drw_textw(lua_State*);
static int l_u_keypress(lua_State*);
static int l_u_newclient(lua_State*);
static int l_u_status_click(lua_State*);
static int l_u_status_draw(lua_State*);
static int l_u_status_text(lua_State*);
static int l_u_systray_width(lua_State*);

struct l_Client {
	Client *c;
};

static int
l_u_client_tag(lua_State *L) {
	struct l_Client *w = luaL_checkudata(L, 1, "dwm-client");
	tag(w->c, (unsigned int) luaL_checkinteger(L, 2));
	return 0;
}

static int
l_u_client_toggletag(lua_State *L) {
	struct l_Client *w = luaL_checkudata(L, 1, "dwm-client");
	toggletag(w->c, (unsigned int) luaL_checkinteger(L, 2));
	return 0;
}

static int
l_u_client_togglefloating(lua_State *L) {
	struct l_Client *w = luaL_checkudata(L, 1, "dwm-client");
	togglefloating(w->c);
	return 0;
}

static int
l_u_client_kill(lua_State *L) {
	struct l_Client *w = luaL_checkudata(L, 1, "dwm-client");
	killclient(w->c);
	return 0;
}

static int
l_u_tag_toggleview(lua_State *L) {
	toggleview((unsigned int) luaL_checkinteger(L, 1));
	return 0;
}

static int
l_u_tag_view(lua_State *L) {
	view((unsigned int) luaL_checkinteger(L, 1));
	return 0;
}

static int
l_u_drw_setscheme(lua_State *L) {
	ClrScheme *newscheme;
	const char *border, *fg, *bg;

	if (lua_gettop(L) != 1)
		return luaL_error(L, "Expected exactly one argument");
	if (!lua_istable(L, -1))
		return luaL_error(L, "Expected a table");

	lua_pushliteral(L, "border");
	lua_gettable(L, 1);
	if (lua_isnil(L, -1)) {
		border = "#000";
	} else if (lua_isstring(L, -1)) {
		border = lua_tolstring(L, -1, NULL);
	} else {
		return luaL_error(L, "Expected a string or no value for \"border\", got a %s",
		                  lua_typename(L, lua_type(L, -1)));
	}

	lua_pushliteral(L, "bg");
	lua_gettable(L, 1);
	typeassert(L, -1, string);
	bg = lua_tolstring(L, -1, NULL);

	lua_pushliteral(L, "fg");
	lua_gettable(L, 1);
	typeassert(L, -1, string);
	fg = lua_tolstring(L, -1, NULL);

	newscheme = calloc(1, sizeof(*newscheme));
	if (!newscheme) {
		return luaL_error(L, "Can't allocate new color scheme");
	}
	newscheme->border = drw_clr_create(drw, border);
	if (!newscheme->border) {
		return luaL_error(L, "Can't allocate border color");
	}
	newscheme->bg = drw_clr_create(drw, bg);
	if (!newscheme->bg) {
		return luaL_error(L, "Can't allocate bg color");
	}
	newscheme->fg = drw_clr_create(drw, fg);
	if (!newscheme->fg) {
		return luaL_error(L, "Can't allocate fg color");
	}

	/* Clean up previously set scheme */
	if (l_scheme) {
		drw_clr_free(l_scheme->border);
		drw_clr_free(l_scheme->bg);
		drw_clr_free(l_scheme->fg);
		free(l_scheme);
	}

	l_scheme = newscheme;
	drw_setscheme(drw, newscheme);

	return 0;
}

static int
l_u_drw_text(lua_State *L) {
	int x, w, invert = 0, simple = 0;
	const char *txt;

	/* Params: x, w, text, invert?, simple? */
	if (lua_gettop(L) < 3 || lua_gettop(L) > 5)
		return luaL_error(L, "Unexpected number of arguments: %d", lua_gettop(L));

	typeassert(L, 1, number);
	x = lua_tonumber(L, 1);

	typeassert(L, 2, number);
	w = lua_tointeger(L, 2);

	typeassert(L, 3, string);
	txt = lua_tolstring(L, 3, NULL);

	if (lua_gettop(L) >= 4) {
		typeassert(L, 4, boolean);
		invert = lua_toboolean(L, 4);
	}
	if (lua_gettop(L) == 5) {
		typeassert(L, 4, boolean);
		simple = lua_toboolean(L, 5);
	}

	drw_text(drw, x, 0, w, bh, txt, invert, simple);
	return 0;
}

static int
l_u_drw_textw(lua_State *L) {
	const char *txt;
	size_t len;
	int simple = 0;

	if (lua_gettop(L) > 2) {
		return luaL_error(L, "Expected maximum of two arguments, got %d", lua_gettop(L));
	}
	if (lua_gettop(L) == 0) {
		return luaL_error(L, "Expected at least one argument");
	}

	if (lua_gettop(L) == 2) {
		simple = lua_toboolean(L, 2);
	}

	txt = lua_tolstring(L, 1, &len);
	if (!txt) {
		return luaL_error(L, "Can't convert argument to string");
	}

	if (simple) {
		lua_pushnumber(L, drw_font_getexts_width(drw, txt, len));
	} else {
		lua_pushnumber(L, TEXTW(txt));
	}
	return 1;
}

static int
l_u_status_click(lua_State *L) {
	if (lua_gettop(L) == 0) {
		/* Return draw function, if any */
		lua_pushliteral(L, "dwm-status-clickfn");
		lua_rawget(L, LUA_REGISTRYINDEX);
		return 1;
	}

	if (lua_gettop(L) != 1) {
		return luaL_error(L, "Expected one argument, got %d", lua_gettop(L));
	}
	typeassert(L, -1, function);

	lua_pushliteral(L, "dwm-status-clickfn");
	lua_rotate(L, -2, 1);
	lua_rawset(L, LUA_REGISTRYINDEX);

	return 0;
}

static int
l_u_status_draw(lua_State *L) {
	if (lua_gettop(L) == 0) {
		/* Return draw function, if any */
		lua_pushliteral(L, "dwm-status-drawfn");
		lua_rawget(L, LUA_REGISTRYINDEX);
		return 1;
	}

	if (lua_gettop(L) != 1) {
		return luaL_error(L, "Expected one argument, got %d", lua_gettop(L));
	}
	typeassert(L, -1, function);

	lua_pushliteral(L, "dwm-status-drawfn");
	lua_rotate(L, -2, 1);
	lua_rawset(L, LUA_REGISTRYINDEX);

	return 0;
}

static int
l_u_status_text(lua_State *L) {
	lua_pushstring(L, stext);
	return 1;
}

static int
l_u_systray_width(lua_State *L) {
	lua_pushnumber(L, 10);
	return 1;
}

static int
l_u_tag_click(lua_State *L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "Expected one argument");
	}
	if (!lua_isfunction(L, 1) && !lua_isnil(L, 1)) {
		return luaL_error(L, "Expected either a function or a nil, got a %s", lua_typename(L, lua_type(L, 1)));
	}

	lua_pushliteral(L, "dwm-tag-click");
	lua_rotate(L, -2, 1);
	lua_rawset(L, LUA_REGISTRYINDEX);

	return 0;
}

int
l_call_status_click(int mods, int btn) {
	if (!globalL) {
		return 0;
	}

	lua_pushliteral(globalL, "dwm-status-clickfn");
	if (lua_rawget(globalL, LUA_REGISTRYINDEX) != LUA_TFUNCTION) {
		lua_pop(globalL, 1);
		return 0;
	}

	lua_pushinteger(globalL, mods);
	lua_pushinteger(globalL, btn);

	if (lua_pcall(globalL, 2, 0, 0) != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tolstring(globalL, -1, NULL));
		return 0;
	}

	return 1;
}

int
l_call_tag_click(int mods, int btn, int tag) {
	if (!globalL) {
		return 0;
	}

	lua_pushliteral(globalL, "dwm-tag-click");
	if (lua_rawget(globalL, LUA_REGISTRYINDEX) != LUA_TFUNCTION) {
		lua_pop(globalL, 1);
		return 0;
	}

	lua_pushinteger(globalL, mods);
	lua_pushinteger(globalL, btn);
	lua_pushinteger(globalL, tag);

	if (lua_pcall(globalL, 3, 0, 0) != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tolstring(globalL, -1, NULL));
		return 0;
	}

	return 1;
}

static int
l_open_lib(lua_State *L) {
	struct luaL_Reg tagfuncs[] = {
		{ "click", l_u_tag_click },
		{ "toggleview", l_u_tag_toggleview },
		{ "view", l_u_tag_view },
		{ NULL, NULL },
	};

	struct luaL_Reg drwfuncs[] = {
		{ "textw", l_u_drw_textw },
		{ "text", l_u_drw_text },
		{ "setscheme", l_u_drw_setscheme },
		{ NULL, NULL },
	};

	struct luaL_Reg statusfuncs[] = {
		{ "text", l_u_status_text },
		{ "draw", l_u_status_draw },
		{ "click", l_u_status_click },
		{ NULL, NULL },
	};

	struct luaL_Reg modfuncs[] = {
		{ "systray_width", l_u_systray_width },
		{ "newclient", l_u_newclient },
		{ NULL, NULL },
	};

	struct luaL_Reg clientfuncs_m[] = {
		{ "__index", l_u_client_index },
		{ NULL, NULL },
	};

	struct luaL_Reg clientfuncs_f[] = {
		{ "current", l_u_client_current },
		{ "focusstack", l_u_client_focusstack },
		{ NULL, NULL },
	};

	luaL_newmetatable(L, "dwm-client");
	luaL_setfuncs(L, clientfuncs_m, 0);
	lua_pop(L, 1);

	luaL_newlib(L, modfuncs);

	/* Clients */
	lua_pushliteral(L, "client");
	luaL_newlib(L, clientfuncs_f);
	lua_rawset(L, 2);

	/* Tags */
	lua_pushliteral(L, "tags");
	luaL_newlib(L, tagfuncs);
	lua_rawset(L, 2);

	/* Status */
	lua_pushliteral(L, "status");
	luaL_newlib(L, statusfuncs);
	lua_rawset(L, 2);

	/* Drawing */
	lua_pushliteral(L, "drw");
	luaL_newlib(L, drwfuncs);
	lua_rawset(L, 2);

	/* Keys */
	lua_pushliteral(L, "keys");
	luaL_newlib(L, ((struct luaL_Reg[]){{"press", l_u_keypress}, {NULL, NULL}}));

	lua_pushliteral(L, "mod1");
	lua_pushinteger(L, Mod1Mask);
	lua_rawset(L, -3);

	lua_pushliteral(L, "mod3");
	lua_pushinteger(L, Mod3Mask);
	lua_rawset(L, -3);

	lua_pushliteral(L, "mod4");
	lua_pushinteger(L, Mod4Mask);
	lua_rawset(L, -3);

	lua_pushliteral(L, "shift");
	lua_pushinteger(L, ShiftMask);
	lua_rawset(L, -3);

	lua_pushliteral(L, "control");
	lua_pushinteger(L, ControlMask);
	lua_rawset(L, -3);

	/* Assign 'keys' table to slot in 'dwm' module */
	lua_rawset(L, 2);

	return 1;
}

void
l_loadconfig() {
	char *confname;

	if (!globalL) {
		l_init();
		return;
	}

	/* Load config */
	(void) asprintf(&confname, "%s/.dwm-gbe.lua", getenv("HOME"));

	if (luaL_dofile(globalL, confname)) {
		if (lua_gettop(globalL) >= 1 && lua_isstring(globalL, -1)) {
			printf("%s\n", lua_tolstring(globalL, -1, NULL));
		} else {
			printf("%s: something went wrong, but I don't have an error message.", confname);
			printf("\t%d things are on  the stack\n", lua_gettop(globalL));
		}
	}

	free(confname);
}

void
l_init() {
	lua_State *L = luaL_newstate();
	if (!L) {
		err(1, NULL);
	}
	globalL = L;

	luaL_openlibs(L);
	luaL_requiref(L, "dwm", l_open_lib, 1);

	lua_pushliteral(L, "dwm-keypress");
	lua_newtable(L);
	lua_rawset(L, LUA_REGISTRYINDEX);

	l_loadconfig();
}

int
l_call_status_drawfn(int x, int mw, int sel) {
	int res, isnumber;

	if (!globalL) {
		return -1;
	}

	lua_pushliteral(globalL, "dwm-status-drawfn");
	if (lua_rawget(globalL, LUA_REGISTRYINDEX) != LUA_TFUNCTION) {
		lua_pop(globalL, 1);
		return -1;
	}

	lua_pushinteger(globalL, x);
	lua_pushinteger(globalL, mw);
	lua_pushboolean(globalL, sel);

	if (lua_pcall(globalL, 3, 1, 0) != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tolstring(globalL, -1, NULL));
		return -1;
	}

	if (!lua_isnumber(globalL, -1)) {
		fprintf(stderr, "Got weird return from Lua drawfn: %s, expected a number\n",
		        lua_typename(globalL, lua_type(globalL, -1)));
		lua_pop(globalL, 1);
		return -1;
	}

	res = (int) lua_tonumberx(globalL, -1, &isnumber);

	lua_pop(globalL, 1);

	if (!isnumber)
		return -1;
	return res;
}

static int
l_u_keypress(lua_State *L) {
	int modifiers, type, ungrab;
	const char *symname;
	char *key;
	KeySym ksym;

	if (lua_gettop(L) != 3) {
		return luaL_error(L, "Expected 3 arguments, got %d", lua_gettop(L));
	}
	typeassert(L, 1, integer);
	typeassert(L, 2, string);
	if (!lua_isfunction(L, 3) && !lua_isnil(L, 3)) {
		return luaL_error(L, "Expected either a function or nil as last argument, got a %s!",
		                  lua_typename(L, lua_type(L, 3)));
	}
	ungrab = lua_isnil(L, 3);

	modifiers = (int) lua_tonumber(L, 1);
	symname = lua_tolstring(L, 2, NULL);
	if (!symname) {
		return luaL_error(L, "Can't convert argument 2 to string!");
	}
	ksym = XStringToKeysym(symname);
	if (ksym == NoSymbol) {
		return luaL_error(L, "Key sym \"%s\" is unknown", symname);
	}

	lua_pushliteral(L, "dwm-keypress");
	type = lua_rawget(L, LUA_REGISTRYINDEX);
	assert(type == LUA_TTABLE);

	asprintf(&key, "%x-%s", modifiers, symname);
	if (key == NULL) {
		return luaL_error(L, "Can't allocate space for table key!");
	}

	lua_pushstring(L, key);
	lua_rotate(L, -3, -1); /* If ungrab is true, we've already got a nil on the stack */
	lua_rawset(L, -3);

	free(key);

	grabkey(modifiers, ksym, ungrab);

	return 0;
}

int
l_call_keypress(unsigned int mod, KeySym keysym) {
	int type;
	char *symname, *key;

	if (!globalL) {
		return 0;
	}

	lua_pushliteral(globalL, "dwm-keypress");
	type = lua_rawget(globalL, LUA_REGISTRYINDEX);
	assert(type == LUA_TTABLE);

	symname = XKeysymToString(keysym);
	asprintf(&key, "%x-%s", mod, symname);
	if (!key) {
		fprintf(stderr, "Malloc failure!\n");
		return 0;
	}

	lua_pushstring(globalL, key);
	type = lua_rawget(globalL, -2);
	free(key);

	if (type != LUA_TFUNCTION) {
		fprintf(stderr, "Got something that's a %s, expected a function! (sym='%s')\n",
		        lua_typename(globalL, type), symname);
		return 0;
	}

	lua_pushinteger(globalL, mod);
	lua_pushstring(globalL, symname);

	if (lua_pcall(globalL, 2, 0, 0) != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tolstring(globalL, -1, NULL));
		return 0;
	}

	return 1;
}

int
l_u_newclient(lua_State *L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "Expected one argument, got %d", lua_gettop(L));
	}
	typeassert(L, -1, function);

	lua_pushliteral(L, "dwm-newclient");
	lua_rotate(L, -2, 1);
	lua_rawset(L, LUA_REGISTRYINDEX);

	return 0;
}

static int
l_u_client_index(lua_State *L) {
	struct l_Client *w;
	int prop;
	const char *propnames[] = { "name", "class", "instance", "tag", "toggletag", "togglefloating", "kill", NULL };

	w = luaL_checkudata(L, 1, "dwm-client");

	prop = luaL_checkoption(L, 2, NULL, propnames);
	switch (prop) {
		case 0:
			lua_pushstring(L, w->c->name);
			break;
		case 1:
			lua_pushstring(L, w->c->class);
			break;
		case 2:
			lua_pushstring(L, w->c->instance);
			break;
		case 3:
			lua_pushcfunction(L, l_u_client_tag);
			break;
		case 4:
			lua_pushcfunction(L, l_u_client_toggletag);
			break;
		case 5:
			lua_pushcfunction(L, l_u_client_togglefloating);
			break;
		case 6:
			lua_pushcfunction(L, l_u_client_kill);
			break;
		default:
			return luaL_error(L, "Unknown property requested!");
	}

	return 1;
}

static int
l_u_client_current(lua_State *L) {
	struct l_Client *wrapper;

	if (!selmon || !selmon->sel) {
		lua_pushnil(L);
	} else {
		wrapper = lua_newuserdata(globalL, sizeof(*wrapper));
		wrapper->c = selmon->sel;
		luaL_getmetatable(L, "dwm-client");
		lua_setmetatable(L, -2);
	}

	return 1;
}

static int
l_u_client_focusstack(lua_State *L) {
	int off = luaL_checkinteger(L, 1);
	focusstack(off);
	return 0;
}

int
l_call_newclient(Client *client) {
	struct l_Client *wrapper;

	if (!globalL) {
		return 0;
	}

	lua_pushliteral(globalL, "dwm-newclient");
	if (lua_rawget(globalL, LUA_REGISTRYINDEX) != LUA_TFUNCTION) {
		lua_pop(globalL, 1);
		return 0;
	}

	wrapper = lua_newuserdata(globalL, sizeof(*wrapper));
	wrapper->c = client;

	luaL_getmetatable(globalL, "dwm-client");
	lua_setmetatable(globalL, -2);

	if (lua_pcall(globalL, 1, 0, 0) != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tolstring(globalL, -1, NULL));
		return 0;
	}

	return 1;
}
