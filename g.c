/* See LICENSE file for copyright and license details. */
#include <errno.h>

#include <libguile.h>

#include "util.h"
#include "g.h"
#include "drw.h"

extern int bh;
extern char stext[256];

extern Drw *drw;
extern ClrScheme scheme[SchemeLast];

/* Hook for scheme drawbar */
SCM g_drawstatus_hook = SCM_UNDEFINED;

SCM
g_drw_setscheme(SCM colorscheme) {
	if (!scm_is_symbol(colorscheme)) {
		/* Raise an exception? */
		fprintf(stderr, "Expected a symbol\n");
		return SCM_UNSPECIFIED;
	}

	if (scm_is_eq(colorscheme, scm_from_utf8_symbol("normal"))) {
		drw_setscheme(drw, &scheme[SchemeNorm]);
	} else if (scm_is_eq(colorscheme, scm_from_utf8_symbol("selected"))) {
		drw_setscheme(drw, &scheme[SchemeSel]);
	} else {
		/* TODO: raise exception */
		SCM s_symname = scm_symbol_to_string(colorscheme);
		char *symname = scm_to_utf8_stringn(s_symname, NULL);
		fprintf(stderr, "Unknown color scheme symbol: \"%s\"\n", symname);
		free(symname);
	}
	return SCM_UNSPECIFIED;
}

SCM
g_drw_textw(SCM s_txt, SCM simple) {
	char *txt = scm_to_utf8_stringn(s_txt, NULL);
	int rv;

	if (SCM_UNBNDP(simple))
		simple = SCM_BOOL_F;

	if (scm_to_bool(simple)) {
		rv = drw_font_getexts_width(drw, txt, strlen(txt));
	} else {
		rv = TEXTW(txt);
	}

	free(txt);
	return scm_from_int(rv);
}

SCM
g_drw_text(SCM x, SCM w, SCM s_text, SCM invert, SCM simple) {
	char *txt = scm_to_utf8_stringn(s_text, NULL);
	if (SCM_UNBNDP(invert))
		invert = SCM_BOOL_F;
	if (SCM_UNBNDP(simple))
		simple = SCM_BOOL_F;
	drw_text(drw, scm_to_int(x), 0, scm_to_int(w), bh, txt, scm_to_bool(invert), scm_to_bool(simple));
	free(txt);
	return SCM_UNSPECIFIED;
}

SCM
g_drawstatus_hook_fn(SCM drawfn) {
	/* XXX: Use guile hooks
	 * https://www.gnu.org/software/guile/manual/html_node/Hooks.html
	 */
	if (SCM_UNBNDP(drawfn))
		return g_drawstatus_hook;
	g_drawstatus_hook = drawfn;
	return SCM_UNSPECIFIED;
}

SCM
g_statustext() {
	return scm_from_utf8_string(stext);
}

SCM
g_spawn(SCM s_cmd) {
	Arg arg;
	char **cmdv;
	int i, cmdlen;

	if (!scm_is_true(scm_list_p(s_cmd))) {
		fprintf(stderr, "expected a list\n");
		return SCM_UNSPECIFIED;
	}

	cmdlen = scm_to_int(scm_length(s_cmd));
	cmdv = calloc(cmdlen + 1, sizeof(char*));
	for (i = 0; i < cmdlen; i++) {
		cmdv[i] = scm_to_utf8_stringn(scm_list_ref(s_cmd, scm_from_int(i)), NULL);
	}
	cmdv[cmdlen] = NULL;

	arg.v = cmdv;
	spawn(&arg);

	for (i = 0; i < cmdlen; i++) {
		free(cmdv[i]);
	}
	free(cmdv);

	return SCM_UNSPECIFIED;
}

SCM
g_systraywidth() {
	return scm_from_uint(getsystraywidth());
}

void
g_run_conf(const Arg *arg) {
	/* TODO: handle exceptions while loading */
	char *tmp, *home;
	struct stat sbuf;

	if ((home = getenv("HOME")) == NULL) {
		fprintf(stderr, "Looks like we're homeless :(\n");
		return;
	}

	(void) asprintf(&tmp, "%s/.dwm.scm", getenv("HOME"));

	if (stat(tmp, &sbuf) != 0) {
		fprintf(stderr, "Can't access %s: %s\n", tmp, strerror(errno));
		free(tmp);
		return;
	}

	scm_c_primitive_load(tmp);
	free(tmp);
}

void *
g_init(void *data) {
	scm_c_define_gsubr("dwm-drw-textw", 1, 1, 0, g_drw_textw);
	scm_c_define_gsubr("dwm-drw-text", 3, 2, 0, g_drw_text);
	scm_c_define_gsubr("dwm-status-text", 0, 0, 0, g_statustext);
	scm_c_define_gsubr("dwm-systray-width", 0, 0, 0, g_systraywidth);
	scm_c_define_gsubr("dwm-hook-drawstatus", 0, 1, 0, g_drawstatus_hook_fn);
	scm_c_define_gsubr("dwm-spawn", 0, 0, 1, g_spawn);
	scm_c_define_gsubr("dwm-drw-setscheme", 1, 0, 0, g_drw_setscheme);
	g_run_conf(NULL);
	return NULL;
}
