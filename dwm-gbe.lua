dwm.drawstatus(function (x w sel)
	local s = dwm.status_text()
	local sw = dwm.drw_textw(s)
	local sx = w - sw - dwm.systray_width()
	dwm.drw_text(sx, sw, s)
	return sx
end)

print("Done loading config")
