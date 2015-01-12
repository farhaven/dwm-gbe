/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

	return font;
}

void
drw_font_free(Display *dpy, Fnt *font) {
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
drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, bool invert, bool markup) {
	char buf[256];
	int i, tx, ty, th, len, olen;
	Extnts tex;
	XftDraw *d;

	if(!drw || !drw->scheme)
		return;
	XSetForeground(drw->dpy, drw->gc, (invert? drw->scheme->fg->rgb: drw->scheme->bg->rgb).pixel);
	XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
	if(!text || !drw->font)
		return;
	olen = strlen(text);
	drw_font_getexts(drw, text, olen, &tex, markup);
	th = drw->font->ascent + drw->font->descent;
	ty = y + (h / 2) - (th / 2);
	tx = x + (h / 2);
	/* shorten text if necessary */
	for(len = MIN(olen, sizeof buf); len && (tex.w > w - tex.h || w < tex.h); len--)
		drw_font_getexts(drw->dpy, drw->font, text, len, &tex, markup);
	if(!len)
		return;
	memcpy(buf, text, len);
	if(len < olen)
		for(i = len; i && i > len - 3; buf[--i] = '.');
	XSetForeground(drw->dpy, drw->gc, (invert? drw->scheme->bg->rgb: drw->scheme->fg->rgb).pixel);

	d = XftDrawCreate(drw->dpy, drw->drawable, DefaultVisual(drw->dpy, drw->screen), DefaultColormap(drw->dpy, drw->screen));
	if ((!markup) || (!pango_parse_markup(buf, len, 0, NULL, NULL, NULL, NULL))) {
		pango_layout_set_markup(drw->font->plo, "", 0);
		pango_layout_set_text(drw->font->plo, buf, len);
	} else {
		pango_layout_set_text(drw->font->plo, "", 0);
		pango_layout_set_markup(drw->font->plo, buf, len);
	}
	pango_xft_render_layout(d, &(invert? drw->scheme->bg: drw->scheme->fg)->rgb,
			drw->font->plo, tx * PANGO_SCALE, ty * PANGO_SCALE);
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
drw_font_getexts(Drw* drw, const char *text, unsigned int len, Extnts *tex, bool markup) {
	PangoRectangle r;
	if ((!markup) || (!pango_parse_markup(text, len, 0, NULL, NULL, NULL, NULL))) {
		pango_layout_set_markup(drw->font->plo, "", 0);
		pango_layout_set_text(drw->font->plo, text, len);
	} else {
		pango_layout_set_text(drw->font->plo, "", 0);
		pango_layout_set_markup(drw->font->plo, text, len);
	}
	pango_layout_get_extents(drw->font->plo, &r, 0);
	tex->w = r.width / PANGO_SCALE;
	tex->h = r.height / PANGO_SCALE;
}

unsigned int
drw_font_getexts_width(Drw* drw, const char *text, unsigned int len, bool markup) {
	Extnts tex;

	if(!drw->font)
		return -1;
	drw_font_getexts(drw, text, len, &tex, markup);
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
