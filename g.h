/* See LICENSE file for copyright and license details. */
SCM  g_drawstatus_hook_fn(SCM);
SCM  g_drw_setscheme(SCM);
SCM  g_drw_text(SCM, SCM, SCM, SCM, SCM);
SCM  g_drw_textw(SCM, SCM);
SCM  g_getsystraywidth(void);
SCM  g_spawn(SCM);
SCM  g_statustext(void);

void  g_run_conf(const Arg *arg);
void *g_init(void*);

extern SCM g_drawstatus_hook;
