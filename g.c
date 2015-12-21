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

ClrScheme scheme_now;

/* Hook for scheme drawbar */
SCM g_drawstatus_hook = SCM_UNDEFINED;

struct g_ClrScheme {
	ClrScheme *s;
	SCM update_func;
	SCM fg;
	SCM bg;
	SCM border;
};
static scm_t_bits g_ClrScheme_tag;

SCM
g_make_clrscheme(SCM s_fg, SCM s_bg, SCM s_border) {
	char *fg, *bg, *border;
	SCM smob;
	struct g_ClrScheme *cscm;

	/* parse fg/bg/border */
	fg = scm_to_utf8_stringn(s_fg, NULL);
	bg = scm_to_utf8_stringn(s_bg, NULL);
	if (SCM_UNBNDP(s_border))
		s_border = scm_from_utf8_string("#000");
	border = scm_to_utf8_stringn(s_border, NULL);

	cscm = scm_gc_malloc(sizeof(*cscm), "colorscheme");
	cscm->update_func = SCM_BOOL_F;
	cscm->s = scm_gc_malloc(sizeof(*cscm->s), "colorscheme wrapped data");
	cscm->s->border = drw_clr_create(drw, border);
	cscm->s->bg = drw_clr_create(drw, bg);
	cscm->s->fg = drw_clr_create(drw, fg);
	cscm->fg = s_fg;
	cscm->bg = s_bg;
	cscm->border = s_border;

	SCM_NEWSMOB(smob, g_ClrScheme_tag, cscm);

	return smob;
}

SCM
g_ClrScheme_mark(SCM s_smob) {
	struct g_ClrScheme *scm = (struct g_ClrScheme*) SCM_SMOB_DATA(s_smob);
	scm_gc_mark(scm->fg);
	scm_gc_mark(scm->bg);
	scm_gc_mark(scm->border);
	return scm->update_func;
}

size_t
g_ClrScheme_free(SCM s_smob) {
	struct g_ClrScheme *scm = (struct g_ClrScheme*) SCM_SMOB_DATA(s_smob);

	drw_clr_free(scm->s->border);
	drw_clr_free(scm->s->bg);
	drw_clr_free(scm->s->fg);

	scm_gc_free(scm->s, sizeof(*scm->s), "colorscheme wrapped data");
	scm_gc_free(scm, sizeof(*scm), "colorscheme");

	return 0;
}

int
g_ClrScheme_print(SCM s_smob, SCM port, scm_print_state *pstate) {
	struct g_ClrScheme *scm = (struct g_ClrScheme*) SCM_SMOB_DATA(s_smob);

	scm_puts("#<DWM color scheme bg=", port);
	scm_display(scm->bg, port);
	scm_puts(", fg=", port);
	scm_display(scm->fg, port);
	scm_puts(", border=", port);
	scm_display(scm->border, port);
	scm_puts(">", port);
	return 1;
}

SCM
g_drw_setscheme(SCM colorscheme) {
	size_t sz;
	struct g_ClrScheme *scm;

	scm_assert_smob_type(g_ClrScheme_tag, colorscheme);
	scm = (struct g_ClrScheme*) SCM_SMOB_DATA(colorscheme);

	drw_clr_free(scheme_now.border);
	drw_clr_free(scheme_now.bg);
	drw_clr_free(scheme_now.fg);

	sz = sizeof(*scheme_now.border);
	scheme_now.border = calloc(1, sz);
	scheme_now.bg = calloc(1, sz);
	scheme_now.fg = calloc(1, sz);

	memcpy(scheme_now.border, scm->s->border, sz);
	memcpy(scheme_now.bg, scm->s->bg, sz);
	memcpy(scheme_now.fg, scm->s->fg, sz);

	scm_remember_upto_here_1(colorscheme);

	drw_setscheme(drw, &scheme_now);

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

	(void) asprintf(&tmp, "%s/.dwm-gbe.scm", getenv("HOME"));

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
	/* Function bindings */
	scm_c_define_gsubr("dwm-drw-textw", 1, 1, 0, g_drw_textw);
	scm_c_define_gsubr("dwm-drw-text", 3, 2, 0, g_drw_text);
	scm_c_define_gsubr("dwm-status-text", 0, 0, 0, g_statustext);
	scm_c_define_gsubr("dwm-systray-width", 0, 0, 0, g_systraywidth);
	scm_c_define_gsubr("dwm-hook-drawstatus", 0, 1, 0, g_drawstatus_hook_fn);
	scm_c_define_gsubr("dwm-spawn", 0, 0, 1, g_spawn);
	scm_c_define_gsubr("dwm-drw-set-colorscheme", 1, 0, 0, g_drw_setscheme);

	/* Set up data structures */
	memset(&scheme_now, 0x00, sizeof(scheme_now));
	g_ClrScheme_tag = scm_make_smob_type("colorscheme", sizeof(struct g_ClrScheme));
	scm_set_smob_mark(g_ClrScheme_tag, g_ClrScheme_mark);
	scm_set_smob_free(g_ClrScheme_tag, g_ClrScheme_free);
	scm_set_smob_print(g_ClrScheme_tag, g_ClrScheme_print);
	scm_c_define_gsubr("dwm-make-colorscheme", 2, 1, 0, g_make_clrscheme);

	/* Finally, run user configuration */
	scm_setlocale(scm_variable_ref(scm_c_lookup("LC_ALL")), scm_from_locale_string(""));
	g_run_conf(NULL);
	return NULL;
}
