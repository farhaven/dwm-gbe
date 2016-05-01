dwm-gbe - dynamic window manager
============================
dwm-gbe is an extremely fast, small, and dynamic window manager for X, forked from suckless.org's excellent dwm window manager.


Requirements
------------
In order to build dwm you need the Xlib header files.


Installation
------------
Edit config.mk to match your local setup (dwm is installed into
the /usr/local namespace by default).

Afterwards enter the following command to build and install dwm (if
necessary as root):

    make clean install

If you are going to use the default bluegray color scheme it is highly
recommended to also install the bluegray files shipped in the dextra package.


Running dwm
-----------
Add the following line to your .xinitrc to start dwm using startx:

    exec dwm

In order to connect dwm to a specific display, make sure that
the DISPLAY environment variable is set correctly, e.g.:

    DISPLAY=foo.bar:1 exec dwm

(This will start dwm on display :1 of the host foo.bar.)

In order to display status info in the bar, you can do something
like this in your .xinitrc:

    while xsetroot -name "`date` `uptime | sed 's/.*,//'`"
    do
    	sleep 1
    done &
    exec dwm


Configuration
-------------
The configuration of dwm is done by creating a custom config.h
and (re)compiling the source code.

Lua bindings
------------
This variant of dwm has bindings to Lua 5.3 which are in an embryionic state
at the moment. At startup, dwm executes `~/.dwm-gbe.lua` if it exists. The
following functions are available to Lua:

* `dwm.status_text` returns the name of the root window. This is what will be
  printed on the right of the status bar in regular DWM.
* `dwm.drw_textw txt [simple=false]` returns the widths in pixels the text
  `txt` would occupy if drawn using the current font. If the optional parameter
  `simple` is set to `false` (defaults to `false`), the returned width does not
  include additional border space.
* `dwm.systray_width` returns the width required to draw the systray icons.
* `dwm.drw_text x w txt [invert=false] [simple=false]` draws the text `txt` at
  x-offset `x`, so that it occupies `w` pixels at the maximum. If the text were
  longer, it is shortened and `...` is appended. If `invert` is `true` (defaults
  to `false`), the text is drawn with foreground and background color switched.
  If `simple` is `true` (defaults to `false`), there is no additional border
  space inside the drawn space.
* `dwm.drw_setscheme {["bg"]=bg, ["fg"]=fg, ["border"]=border}` sets the
  current color scheme to the specified colors. If `border` is omitted, "#000"
  is assumed. Colors are hex-strings.
* `dwm.drawstatus fn` registers `fn` with signature `(x w s) -> x` as a function
  that draws the status area. The parameter `x` is the right most part of the
  layout icon, the parameter `w` is the maximum available horizontal space
  including the tag and layout icons. If the parameter `s` is true, then the
  selected client is on the screen for which the bar will be redrawn. The
  function returns the x coordinate of the left most pixel it touched. This is
  an example:

```lua
    dwm.drawstatus(function (x w sel)
        local s = dwm.status_text()
        local sw = dwm.drw_textw(s)
        local sx = w - sw - dwm.systray_width()
        dwm.drw_text(sx, sw, s)
    end)
```

The function `l_loadconfig` loads and runs an initial configuration from
`~/.dwm-gbe.lua`. It can be bound to a key binding to reload the
configuration.

Differences from suckless.org's dwm
-----------------------------------
This dwm is a bit different from the one that can be found on suckless.org. It's based on suckless dwm 6.0 with these changes:

* XFT instead of X bitmap fonts. This means unicode support and antialiased fonts
* Small changes to the TEXTW macro to make box sizes symmetric and prevent overdraw of text, for example for long window names
* The `push` patch has been merged into dwm.c
* The `systray` patch has been integrated
* The status bar is exactly as high as required by the font
* There are prototype bindings to Lua 5.3.
* Some small cleanups around the code
