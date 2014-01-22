/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

Drw *
drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h) {
	Drw *drw = (Drw *)calloc(1, sizeof(Drw));
	if(!drw)
		return NULL;
	drw->dpy = dpy;
	drw->screen = screen;
	drw->root = root;
	drw->w = w;
	drw->h = h;
	drw->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
	drw->gc = XCreateGC(dpy, root, 0, NULL);
	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);
	return drw;
}

void
drw_resize(Drw *drw, unsigned int w, unsigned int h) {
	if(!drw)
		return;
	drw->w = w;
	drw->h = h;
	if(drw->drawable != 0)
		XFreePixmap(drw->dpy, drw->drawable);
	drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, DefaultDepth(drw->dpy, drw->screen));
}

void
drw_free(Drw *drw) {
	XFreePixmap(drw->dpy, drw->drawable);
	XFreeGC(drw->dpy, drw->gc);
	free(drw);
}

Fnt *
drw_font_create(Display *dpy, int screen, const char *fontname) {
	Fnt *font;
	PangoFontMetrics *metrics;

	font = (Fnt *)calloc(1, sizeof(Fnt));
	if(!font)
		return NULL;

	font->ctx = pango_xft_get_context(dpy, screen);
	font->pfd = pango_font_description_from_string(fontname);

	metrics = pango_context_get_metrics(font->ctx, font->pfd,
			pango_language_from_string(setlocale(LC_CTYPE, "")));
	font->ascent = pango_font_metrics_get_ascent(metrics) / PANGO_SCALE;
	font->descent = pango_font_metrics_get_descent(metrics) / PANGO_SCALE;
	font->h = font->ascent + font->descent;
	pango_font_metrics_unref(metrics);

	font->plo = pango_layout_new(font->ctx);
	pango_layout_set_font_description(font->plo, font->pfd);
	pango_layout_set_ellipsize(font->plo, PANGO_ELLIPSIZE_END);

	return font;
}

void
drw_font_free(Fnt *font) {
	free(font);
}

Clr *
drw_clr_create(Drw *drw, const char *clrname) {
	Clr *clr;
	Colormap cmap;
	XftColor color;

	if(!drw)
		return NULL;
	clr = (Clr *)calloc(1, sizeof(Clr));
	if(!clr)
		return NULL;
	cmap = DefaultColormap(drw->dpy, drw->screen);
	if(!XftColorAllocName(drw->dpy, DefaultVisual(drw->dpy, drw->screen), cmap, clrname, &color))
		die("error, cannot allocate color '%s'\n", clrname);
	clr->rgb = color;
	return clr;
}

void
drw_clr_free(Clr *clr) {
	free(clr);
}

void
drw_setfont(Drw *drw, Fnt *font) {
	if(drw)
		drw->font = font;
}

void
drw_setscheme(Drw *drw, ClrScheme *scheme) {
	if(drw && scheme) 
		drw->scheme = scheme;
}

void
drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, int filled, int empty, int invert) {
	int dx;

	if(!drw || !drw->font || !drw->scheme)
		return;
	XSetForeground(drw->dpy, drw->gc, (invert? drw->scheme->bg->rgb: drw->scheme->fg->rgb).pixel);
	dx = (drw->font->ascent + drw->font->descent + 2) / 4;
	if(filled)
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x+1, y+1, dx+1, dx+1);
	else if(empty)
		XDrawRectangle(drw->dpy, drw->drawable, drw->gc, x+1, y+1, dx, dx);
}

void
drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, int invert) {
	int tx, ty, th, len;
	Extnts tex;
	XftDraw *d;

	if(!drw || !drw->scheme)
		return;
	XSetForeground(drw->dpy, drw->gc,
			(invert? drw->scheme->fg->rgb: drw->scheme->bg->rgb).pixel);
	XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
	if(!text || !drw->font)
		return;
	XSetForeground(drw->dpy, drw->gc,
			(invert? drw->scheme->bg->rgb: drw->scheme->fg->rgb).pixel);

	th = drw->font->ascent + drw->font->descent;
	ty = (y + (h / 2) - (th / 2)) * PANGO_SCALE;
	tx = (x + (h / 2)) * PANGO_SCALE;

	len = strlen(text);
	drw_font_getexts(drw->font, text, len, &tex);

	w = MIN(w - (tex.h / 2), tex.w) * PANGO_SCALE;

	d = XftDrawCreate(drw->dpy, drw->drawable,
			DefaultVisual(drw->dpy, drw->screen),
			DefaultColormap(drw->dpy, drw->screen));
	pango_layout_set_width(drw->font->plo, w);
	pango_layout_set_text(drw->font->plo, text, len);
	pango_xft_render_layout(d,
			&(invert? drw->scheme->bg: drw->scheme->fg)->rgb,
			drw->font->plo, tx, ty);
	XftDrawDestroy(d);
}

void
drw_map(Drw *drw, Window win, int x, int y, unsigned int w, unsigned int h) {
	if(!drw)
		return;
	XCopyArea(drw->dpy, drw->drawable, win, drw->gc, x, y, w, h, x, y);
	XSync(drw->dpy, False);
}


void
drw_font_getexts(Fnt *font, const char *text, unsigned int len, Extnts *tex) {
	PangoRectangle r;
	pango_layout_set_width(font->plo, -1);
	pango_layout_set_text(font->plo, text, len);
	pango_layout_get_extents(font->plo, NULL, &r);
	tex->w = (r.width - r.x) / PANGO_SCALE;
	tex->h = (r.height - r.y) / PANGO_SCALE;
}

unsigned int
drw_font_getexts_width(Fnt *font, const char *text, unsigned int len) {
	Extnts tex;

	if(!font)
		return -1;
	drw_font_getexts(font, text, len, &tex);
	return tex.w;
}

Cur *
drw_cur_create(Drw *drw, int shape) {
	Cur *cur = (Cur *)calloc(1, sizeof(Cur));

	if(!drw || !cur)
		return NULL;
	cur->cursor = XCreateFontCursor(drw->dpy, shape);
	return cur;
}

void
drw_cur_free(Drw *drw, Cur *cursor) {
	if(!drw || !cursor)
		return;
	XFreeCursor(drw->dpy, cursor->cursor);
	free(cursor);
}
