/* See LICENSE file for copyright and license details. */
#include <err.h>
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

	font = (Fnt *)calloc(1, sizeof(Fnt));
	if(!font)
		return NULL;

	font->xfont = XftFontOpenName(dpy, screen, fontname);
	if (font->xfont == NULL) {
		fprintf(stderr, "Falling back to fixed font\n");
		font->xfont = XftFontOpenName(dpy, screen, "fixed");
	}
	if (font->xfont == NULL) {
		free(font);
		return NULL;
	}

	font->ascent = font->xfont->ascent;
	font->descent = font->xfont->descent;
	font->h = font->ascent + font->descent;

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
	if(!XftColorAllocName(drw->dpy, DefaultVisual(drw->dpy, drw->screen), cmap, clrname, &color)) {
		warnx("error, cannot allocate color '%s'", clrname);
		return NULL;
	}
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
	dx = (drw->font->ascent + drw->font->descent) / 4;
	if(filled)
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x+1, y+1, dx+1, dx+1);
	else if(empty)
		XDrawRectangle(drw->dpy, drw->drawable, drw->gc, x+1, y+1, dx, dx);
}

void
drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, bool invert, bool simple) {
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
	th = drw->font->ascent + drw->font->descent;
	ty = y + (h / 2) - (th / 2) + drw->font->ascent;
	tx = x;
	if (!simple) {
		tx += h / 2;

		/* shorten text if necessary */
		len = MIN(olen, sizeof buf) + 1;
		do {
			drw_font_getexts(drw, text, len--, &tex);
		} while (len && tex.w >= w);
		if(!len)
			return;
		memcpy(buf, text, len);
		if(len < olen)
			for(i = len; i && i > len - 3; buf[--i] = '.');
	} else {
		len = MIN(olen, sizeof buf);
		memset(buf, 0x00, sizeof(buf));
		memcpy(buf, text, len);
	}

	XSetForeground(drw->dpy, drw->gc, (invert? drw->scheme->bg->rgb: drw->scheme->fg->rgb).pixel);

	d = XftDrawCreate(drw->dpy, drw->drawable, DefaultVisual(drw->dpy, drw->screen), DefaultColormap(drw->dpy, drw->screen));
	XftDrawStringUtf8(d, &(invert? drw->scheme->bg: drw->scheme->fg)->rgb, drw->font->xfont, tx, ty, (XftChar8*) buf, len);
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
drw_font_getexts(Drw* drw, const char *text, unsigned int len, Extnts *tex) {
	XGlyphInfo ext;
	XftTextExtentsUtf8(drw->dpy, drw->font->xfont, (XftChar8*) text, len, &ext);
	tex->w = ext.xOff;
	tex->h = ext.yOff;
}

unsigned int
drw_font_getexts_width(Drw* drw, const char *text, unsigned int len) {
	Extnts tex;

	if(!drw->font)
		return -1;
	drw_font_getexts(drw, text, len, &tex);
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
