ifndef PREFIX
	PREFIX = /usr/local
endif
ifndef BINPATH
	BINPATH = ${PREFIX}/bin
endif
ifndef ICONPATH
	ICONPATH = ${PREFIX}/share/isomaster/icons
endif

GTKLIBS = `pkg-config --libs gtk+-2.0`
GTKFLAGS = `pkg-config --cflags gtk+-2.0`
GLOBALFLAGS = -Wall -g
GLOBALDEPS = bk/bk.h

isomaster: lib iniparser isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o $(GLOBALDEPS)
	cc isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o bk/bk.a iniparser-2.15/libiniparser.a $(GTKLIBS) $(GLOBALFLAGS) -o isomaster

lib: 
	cd bk && $(MAKE)

isomaster.o: isomaster.c $(GLOBALDEPS) 
	cc isomaster.c -DICONPATH=\"$(ICONPATH)\" $(GTKFLAGS) $(GLOBALFLAGS) -c

window.o: window.c window.h $(GLOBALDEPS)
	cc window.c $(GTKFLAGS) $(GLOBALFLAGS) -c

browser.o: browser.c $(GLOBALDEPS)
	cc browser.c $(GTKFLAGS) $(GLOBALFLAGS) -c

fsbrowser.o: fsbrowser.c fsbrowser.h $(GLOBALDEPS)
	cc fsbrowser.c $(GTKFLAGS) $(GLOBALFLAGS) -c

isobrowser.o: isobrowser.c isobrowser.h $(GLOBALDEPS)
	cc isobrowser.c $(GTKFLAGS) $(GLOBALFLAGS) -c

error.o: error.c error.h $(GLOBALDEPS)
	cc error.c $(GTKFLAGS) $(GLOBALFLAGS) -c

about.o: about.c about.h $(GLOBALDEPS)
	cc about.c $(GTKFLAGS) $(GLOBALFLAGS) -c

settings.o: settings.c settings.h $(GLOBALDEPS)
	cc settings.c $(GTKFLAGS) $(GLOBALFLAGS) -c

boot.o: boot.c boot.h $(GLOBALDEPS)
	cc boot.c $(GTKFLAGS) $(GLOBALFLAGS) -c

iniparser:
	cd iniparser-2.15 && $(MAKE)

testread2: testread2.c lib
	cc testread2.c bk/bk.a -Wall -o testread2

clean: 
	rm -f *.o isomaster
	cd bk && $(MAKE) clean
	cd iniparser-2.15 && $(MAKE) clean

# for info about DESTDIR see http://www.gnu.org/prep/standards/html_node/DESTDIR.html

install: isomaster
	install -d $(DESTDIR)$(BINPATH)
	install isomaster $(DESTDIR)$(BINPATH)
	install -d $(DESTDIR)$(ICONPATH)
	install -m 644 icons/isomaster.svg $(DESTDIR)$(ICONPATH)
	install -m 644 icons/folder-new.png $(DESTDIR)$(ICONPATH)

uninstall: 
	rm -f $(DESTDIR)$(BINPATH)/isomaster
	rm -f $(DESTDIR)$(ICONPATH)/isomaster.svg
	rm -f $(DESTDIR)$(ICONPATH)/folder-new.png
