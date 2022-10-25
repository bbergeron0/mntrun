# See LICENSE file for copyright and license details.
.POSIX:

VERSION ?= 0.1

PREFIX ?= /usr/local
MANPREFIX ?= $(PREFIX)/share/man

CFLAGS = -Wall -pedantic -DMNTRUN_VERSION=\"$(VERSION)\" -std=c99

mntrun: main.c mount.c
	$(CC) $(CFLAGS) main.c -o mntrun

clean:
	rm mntrun

install: mntrun
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f mntrun $(DESTDIR)$(PREFIX)/bin
	chmod 4755 $(DESTDIR)$(PREFIX)/bin/mntrun
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < mntrun.1 > $(DESTDIR)$(MANPREFIX)/man1/mntrun.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/mntrun.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/mntrun
	rm -f $(DESTDIR)$(MANPREFIX)/man1/mntrun.1

.PHONY: clean install uninstall
