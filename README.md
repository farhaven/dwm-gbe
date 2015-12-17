dwm - dynamic window manager
============================
dwm is an extremely fast, small, and dynamic window manager for X.


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

Guile bindings
--------------
This variant of DWM has bindings to GNU Guile 2.0 which are in an embryionic state at the moment. At startup, DWM executes
`~/.dwm.scm` if it exists. The following functions are available to Guile:

* `dwm-spawn` for spawning external commands:
  `(dwm-spawn "notify-send" "foobar")`
* `dwm-status-text` returns the name of the root window which is to be set as the status bar text
* `dwm-drw-textw txt [simple #f]` returns the widths in pixels the text `txt` would occupy if drawn using the current font. If the
  optional parameter `simple` is set to `#t` (defaults to `#f`), the returned width does not include additional border space.
* `dwm-systray-width` returns the width required to draw the systray icons.
* `dwm-drw-text x w txt [invert #f] [simple #f]` draws the text `txt` at x-offset `x`, so that it occupies `w` pixels at the
  maximum. If the text were longer, it is shortened and `...` is appended. If `invert` is `#t` (defaults to `#f`), the text is drawn
  with foreground and background color switched. If `simple` is `#t` (defaults to `#f`), there is no additional border space inside
  the drawn space.
* `dwm-make-colorscheme fg bg [border]` can be used to create an opaque color scheme to pass to `dwm-drw-set-colorscheme`. If
  `border` is ommitted, `"#000"` is assumed. All colors are hex strings.
* `dwm-drw-set-colorscheme s` can be used to set the current color scheme to `s`.
* `dwm-hook-drawstatus fn` registers `fn` with signature `(x w s) -> x` as a function that draws the status area. The parameter `x`
  is the right most part of the layout icon, the parameter `w` is the maximum available horizontal space including the tag and
  layout icons. If the parameter `s` is true, then the selected client is on the screen for which the bar will be redrawn. The
  function returns the x coordinate of the left most pixel it touched. This is an example:

```scheme
    (dwm-hook-drawstatus
      (lambda (x w sel)
        (let* ((s (dwm-status-text))
               (sw (dwm-drw-textw s))
               (sx (- w sw (dwm-systray-width))))
          (dwm-drw-text sx sw s)
          sx)))
```          

The function `g_run_conf` loads and runs an initial configuration from `~/.dwm.scm`. It can be bound to a key binding to reload the
configuration.

Differences from suckless.org's DWM
-----------------------------------
This DWM is a bit different from the one that can be found on suckless.org. It's based on suckless DWM 6.0 with these changes:

* XFT instead of X bitmap fonts. This means unicode support and antialiased fonts
* Small changes to the TEXTW macro to make box sizes symmetric and prevent overdraw of text, for example for long window names
* The `push` patch has been merged into dwm.c
* The `systray` patch has been integrated
* The status bar is exactly as high as required by the font
* There are prototype bindings to guile.
* Some small cleanups around the code
