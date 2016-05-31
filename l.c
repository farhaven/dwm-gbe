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

/* Missing:
	dwm-spawn
 */

lua_State *globalL = NULL;
ClrScheme *l_scheme = NULL;

extern char stext[256];
extern int bh;
extern Drw *drw;

static int l_u_drawstatus(lua_State*);
static int l_u_drw_setscheme(lua_State*);
static int l_u_drw_text(lua_State*);
static int l_u_drw_textw(lua_State*);
static int l_u_keypress(lua_State*);
static int l_u_status_text(lua_State*);
static int l_u_systray_width(lua_State*);

struct luaL_Reg libfuncs[] = {
	{ "drw_textw", l_u_drw_textw },
	{ "drw_text", l_u_drw_text },
	{ "systray_width", l_u_systray_width },
	{ "status_text", l_u_status_text },
	{ "drawstatus", l_u_drawstatus },
	{ "drw_setscheme", l_u_drw_setscheme },
	{ "keypress", l_u_keypress },
	{ NULL, NULL }
};

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
l_u_drawstatus(lua_State *L) {
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
l_u_systray_width(lua_State *L) {
	lua_pushnumber(L, 10);
	return 1;
}

static int
l_open_lib(lua_State *L) {
	luaL_newlib(L, libfuncs);

	/* TODO: put these into a 'mods' table */
	lua_pushstring(L, "mod4");
	lua_pushinteger(L, Mod4Mask);
	lua_settable(L, 2);

	lua_pushstring(L, "shift");
	lua_pushinteger(L, ShiftMask);
	lua_settable(L, 2);

	lua_pushstring(L, "control");
	lua_pushinteger(L, ControlMask);
	lua_settable(L, 2);

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

	fprintf(stderr, "Keypress register called, mod=0x%04x, sym=\"%s\", key=\"%s\"\n", modifiers, symname, key);

	lua_pushstring(L, key);
	lua_rotate(L, -3, -1); /* If ungrab is true, we've already got a nil on the stack */
	lua_rawset(L, -3);

	free(key);

	fprintf(stderr, "top=%d\n", lua_gettop(L));

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
		fprintf(stderr, "Got something that's a %s, expected a function!\n", lua_typename(globalL, type));
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
