# dwm version
VERSION = 6.1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# includes and libs
INCS = `guile2-config compile`
LIBS = `guile2-config link`

INCS += -I${X11INC} `pkg-config --cflags xft pango pangoxft`
LIBS += -L${X11LIB} -lX11 ${XINERAMALIBS} `pkg-config --libs xft pango pangoxft`

# flags
CPPFLAGS = -D_BSD_SOURCE -D_POSIX_C_SOURCE=2 -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = -g
CFLAGS   += -std=c99 -Wall -Werror -Wno-variadic-macros -Wno-deprecated-declarations ${INCS} ${CPPFLAGS}
LDFLAGS  = -g ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc
