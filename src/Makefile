# other paths are relative to this
ifndef PREFIX
	PREFIX = /usr/local
endif

# where to install the executable
ifndef BINPATH
	BINPATH = ${PREFIX}/bin
endif

# where to install the icons
ifndef ICONPATH
	ICONPATH = ${PREFIX}/share/isomaster/icons
endif

# Used by gettext and po/Makefile (translations)
# To disable installation of .mo files edit or delete the MOFILES 
# variable in po/Makefile. This is safe to do, except obviously
# you won't a have translated ISO Master.
ifndef LOCALEDIR
	export LOCALEDIR = ${PREFIX}/share/locale
endif

GTKLIBS = `pkg-config --libs gtk+-2.0`
GTKFLAGS = `pkg-config --cflags gtk+-2.0`
# the _FILE_OFFSET_BITS=64 is to enable stat() for large files
GLOBALFLAGS = -D_FILE_OFFSET_BITS=64 -Wall
GLOBALDEPS = bk/bk.h Makefile

isomaster: lib iniparser translations isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o $(GLOBALDEPS)
	cc isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o bk/bk.a iniparser-2.15/libiniparser.a $(GTKLIBS) $(GLOBALFLAGS) -o isomaster

lib:
	cd bk && $(MAKE)

iniparser:
	cd iniparser-2.15 && $(MAKE)

translations:
	cd po && $(MAKE)

isomaster.o: isomaster.c $(GLOBALDEPS) 
	cc isomaster.c -DICONPATH=\"$(ICONPATH)\" -DLOCALEDIR=\"$(LOCALEDIR)\" $(GLOBALFLAGS) $(GTKFLAGS) -c

window.o: window.c window.h $(GLOBALDEPS)
	cc window.c $(GLOBALFLAGS) $(GTKFLAGS) -c

browser.o: browser.c $(GLOBALDEPS)
	cc browser.c $(GLOBALFLAGS) $(GTKFLAGS) -c

fsbrowser.o: fsbrowser.c fsbrowser.h $(GLOBALDEPS)
	cc fsbrowser.c $(GLOBALFLAGS) $(GTKFLAGS) -c

isobrowser.o: isobrowser.c isobrowser.h $(GLOBALDEPS)
	cc isobrowser.c $(GLOBALFLAGS) $(GTKFLAGS) -c

error.o: error.c error.h $(GLOBALDEPS)
	cc error.c $(GLOBALFLAGS) $(GTKFLAGS) -c

about.o: about.c about.h $(GLOBALDEPS)
	cc about.c $(GLOBALFLAGS) $(GTKFLAGS) -c

settings.o: settings.c settings.h $(GLOBALDEPS)
	cc settings.c $(GLOBALFLAGS) $(GTKFLAGS) -c

boot.o: boot.c boot.h $(GLOBALDEPS)
	cc boot.c $(GLOBALFLAGS) $(GTKFLAGS) -c

clean: 
	rm -f *.o isomaster
	cd bk && $(MAKE) clean
	cd iniparser-2.15 && $(MAKE) clean
	cd po && make clean

# for info about DESTDIR see http://www.gnu.org/prep/standards/html_node/DESTDIR.html

install: isomaster
	install -d $(DESTDIR)$(BINPATH)
	install isomaster $(DESTDIR)$(BINPATH)
	install -d $(DESTDIR)$(ICONPATH)
	install -m 644 icons/isomaster.png $(DESTDIR)$(ICONPATH)
	install -m 644 icons/folder-new.png $(DESTDIR)$(ICONPATH)
	cd po && $(MAKE) install

uninstall: 
	rm -f $(DESTDIR)$(BINPATH)/isomaster
	rm -f $(DESTDIR)$(ICONPATH)/isomaster.png
	rm -f $(DESTDIR)$(ICONPATH)/folder-new.png
	cd po && $(MAKE) uninstall
