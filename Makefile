CC     ?= CC
OUTPUT  = mayflower
SOURCES = draw.c mayflower.c util/strtonum.c util/util.c wayland/wlr-layer-shell-unstable-v1.c wayland/xdg-shell-protocol.c
HEADERS = config.h draw.h util/util.h wayland/wlr-layer-shell-unstable-v1.h wayland/xdg-shell-client-protocol.h

CFLAGS  = -g -Os -std=c99 -pedantic -Wall -Wextra -Wno-unused-variable -lcairo -lwayland-client -lwayland-cursor -lpthread

PREFIX ?= /usr/local

all: $(OUTPUT)

$(OUTPUT): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SOURCES)

config.h: config.def.h
	cp -i config.def.h config.h

wayland/wlr-layer-shell-unstable-v1.c: wayland/wlr-layer-shell-unstable-v1.xml
	wayland-scanner private-code wayland/wlr-layer-shell-unstable-v1.xml wayland/wlr-layer-shell-unstable-v1.c

wayland/wlr-layer-shell-unstable-v1.h: wayland/wlr-layer-shell-unstable-v1.xml
	wayland-scanner client-header wayland/wlr-layer-shell-unstable-v1.xml wayland/wlr-layer-shell-unstable-v1.h

wayland/xdg-shell-protocol.c: /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
	wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml wayland/xdg-shell-protocol.c

wayland/xdg-shell-client-protocol.h: /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
	wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml wayland/xdg-shell-client-protocol.h

install: $(OUTPUT)
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f $(OUTPUT) ${DESTDIR}${PREFIX}/bin

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/$(OUTPUT)

clean:
	rm -f $(OUTPUT) *.o wayland/wlr-layer-shell-unstable-v1.c wayland/wlr-layer-shell-unstable-v1.h wayland/xdg-shell-client-protocol.h wayland/xdg-shell-protocol.c

.PHONY: all install uninstall clean
