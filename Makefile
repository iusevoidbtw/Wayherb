.POSIX:

cc      = cc
output  = wayherb
sources = util.c wayherb.c wayland.c wlr-layer-shell-unstable-v1.c xdg-shell-protocol.c
headers = config.h util.h wayland.h wlr-layer-shell-unstable-v1.h xdg-shell-client-protocol.h

cflags  = -lcairo -lwayland-client -lwayland-cursor

$(output): $(sources) $(headers)
	$(cc) $(cflags) -o $(output) $(sources)
wlr-layer-shell-unstable-v1.c: wlr-layer-shell-unstable-v1.xml
	wayland-scanner private-code wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1.c
wlr-layer-shell-unstable-v1.h: wlr-layer-shell-unstable-v1.xml
	wayland-scanner client-header wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1.h
xdg-shell-protocol.c: /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
	wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
xdg-shell-client-protocol.h: /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
	wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
clean:
	rm -f $(output) *.o wlr-layer-shell-unstable-v1.c wlr-layer-shell-unstable-v1.h xdg-shell-client-protocol.h xdg-shell-protocol.c
