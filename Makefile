prefix=/usr
datadir=$(prefix)/share
libdir=$(prefix)/lib

all: module-systray.so po/ru/evolution-systray.mo

CFLAGS := -fPIC $(shell pkg-config --cflags evolution-shell-3.0 statusnotifier)
LIBS := $(shell pkg-config --libs evolution-mail-3.0 statusnotifier)

module-systray.so: systray.o
	gcc -shared systray.o -o module-systray.so $(LIBS)

systray.o: systray.c
	gcc $(CFLAGS) -c systray.c -o systray.o

po/ru/evolution-systray.mo: po/ru/evolution-systray.po
	msgfmt --output-file=$@ $<

po/ru/evolution-systray.po: po/evolution-systray.pot
	msgmerge --update $@ $<

po/evolution-systray.pot: systray.c
	xgettext -k_ -j -lC -c -s -o po/evolution-systray.pot systray.c

.PHONY: install clean

install: all
	cp module-systray.so $(libdir)/evolution/modules/
	cp po/ru/*.mo $(datadir)/locale/ru/LC_MESSAGES/

clean:
	rm -f systray.o module-systray.so po/ru/*.mo
