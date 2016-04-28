#include <err.h>
#include <stdlib.h>
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "drw.h"
#include "l.h"

#define typeassert(L, index, type) \
	if (!lua_is##type(L, index)) { \
		return luaL_error(L, "Expected a " #type ", got a %s", \
		                  lua_typename(L, lua_type(L, index))); \
	}

/* Missing:
	dwm-spawn
	dwm-drw-set-colorscheme
 */

lua_State *globalL = NULL;

extern char stext[256];
extern int bh;
extern Drw *drw;

static int l_u_drw_textw(lua_State*);
static int l_u_systray_width(lua_State*);
static int l_u_status_text(lua_State*);
static int l_u_drawstatus(lua_State*);
static int l_u_drw_text(lua_State*);

struct luaL_Reg libfuncs[] = {
	{ "drw_textw", l_u_drw_textw },
	{ "drw_text", l_u_drw_text },
	{ "systray_width", l_u_systray_width },
	{ "status_text", l_u_status_text },
	{ "drawstatus", l_u_drawstatus },
	{ NULL, NULL }
};

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

	printf("Drawing text '%s' @%d %d\n", txt, x, w);
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
	return 1;
}

void
l_init() {
	char *confname;

	lua_State *L = luaL_newstate();
	if (!L) {
		err(1, NULL);
	}

	luaL_openlibs(L);

	luaL_requiref(L, "dwm", l_open_lib, 1);

	/* Load config */
	(void) asprintf(&confname, "%s/.dwm-gbe.lua", getenv("HOME"));

	if (luaL_dofile(L, confname)) {
		if (lua_gettop(L) >= 1 && lua_isstring(L, -1)) {
			printf("%s\n", lua_tolstring(L, -1, NULL));
		} else {
			printf("%s: something went wrong, but I don't have an error message.", confname);
			printf("\t%d things are on  the stack\n", lua_gettop(L));
		}
	}

	free(confname);

	globalL = L;
}

int
l_have_status_drawfn() {
	int type;

	if (!globalL) {
		return 0;
	}

	lua_pushliteral(globalL, "dwm-status-drawfn");
	type = lua_rawget(globalL, LUA_REGISTRYINDEX);
	lua_pop(globalL, 1);

	if (type != LUA_TFUNCTION) {
		return 0;
	}

	return 1;
}

int
l_call_status_drawfn(int x, int mw, int sel) {
	int res;

	lua_pushliteral(globalL, "dwm-status-drawfn");
	lua_rawget(globalL, LUA_REGISTRYINDEX);

	lua_pushinteger(globalL, x);
	lua_pushinteger(globalL, mw);
	lua_pushboolean(globalL, sel);

	lua_call(globalL, 3, 1); /* XXX: pcall */

	if (!lua_isnumber(globalL, -1)) {
		fprintf(stderr, "Got weird return from Lua drawfn: %s, expected a number\n",
		        lua_typename(globalL, lua_type(globalL, -1)));
		lua_pop(globalL, 1);
		return -1;
	}

	res = (int) lua_tonumber(globalL, -1);

	lua_pop(globalL, 1);

	return res;
}
