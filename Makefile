# dwm-gbe - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c dwm.c l.c
OBJ = ${SRC:.c=.o}

all: options dwm-gbe

options:
	@echo dwm-gbe build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

dwm-gbe: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f dwm-gbe ${OBJ} dwm-gbe-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p dwm-gbe-${VERSION}
	@cp -R LICENSE Makefile README config.def.h config.mk \
		dwm-gbe.1 ${SRC} dwm-gbe-${VERSION}
	@tar -cf dwm-gbe-${VERSION}.tar dwm-gbe-${VERSION}
	@gzip dwm-gbe-${VERSION}.tar
	@rm -rf dwm-gbe-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f dwm-gbe ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/dwm-gbe
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < dwm-gbe.1 > ${DESTDIR}${MANPREFIX}/man1/dwm-gbe.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/dwm-gbe.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/dwm-gbe
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/dwm-gbe.1

.PHONY: all options clean dist install uninstall
