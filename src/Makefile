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

# programs used in the Makefiles:
export CC      = gcc
export AR      = ar
export RM      = rm -f
export INSTALL = install

GTKLIBS = `pkg-config --libs gtk+-2.0`
GTKFLAGS = `pkg-config --cflags gtk+-2.0`
# the _FILE_OFFSET_BITS=64 is to enable stat() for large files
GLOBALFLAGS = -D_FILE_OFFSET_BITS=64 -Wall -pedantic -std=c99
GLOBALDEPS = bk/bk.h Makefile

# some build systems require an 'all' target
all: isomaster

isomaster: lib iniparser translations isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o $(GLOBALDEPS)
	$(CC) isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o bk/bk.a iniparser-2.15/libiniparser.a $(GTKLIBS) $(GLOBALFLAGS) -o isomaster

lib:
	cd bk && $(MAKE)

iniparser:
	cd iniparser-2.15 && $(MAKE)

translations:
	cd po && $(MAKE)

isomaster.o: isomaster.c $(GLOBALDEPS) 
	$(CC) isomaster.c -DICONPATH=\"$(ICONPATH)\" -DLOCALEDIR=\"$(LOCALEDIR)\" $(GLOBALFLAGS) $(GTKFLAGS) -c

window.o: window.c window.h $(GLOBALDEPS)
	$(CC) window.c -DICONPATH=\"$(ICONPATH)\" $(GLOBALFLAGS) $(GTKFLAGS) -c

browser.o: browser.c $(GLOBALDEPS)
	$(CC) browser.c $(GLOBALFLAGS) $(GTKFLAGS) -c

fsbrowser.o: fsbrowser.c fsbrowser.h $(GLOBALDEPS)
	$(CC) fsbrowser.c $(GLOBALFLAGS) $(GTKFLAGS) -c

isobrowser.o: isobrowser.c isobrowser.h $(GLOBALDEPS)
	$(CC) isobrowser.c $(GLOBALFLAGS) $(GTKFLAGS) -c

error.o: error.c error.h $(GLOBALDEPS)
	$(CC) error.c $(GLOBALFLAGS) $(GTKFLAGS) -c

about.o: about.c about.h $(GLOBALDEPS)
	$(CC) about.c $(GLOBALFLAGS) $(GTKFLAGS) -c

settings.o: settings.c settings.h $(GLOBALDEPS)
	$(CC) settings.c $(GLOBALFLAGS) $(GTKFLAGS) -c

boot.o: boot.c boot.h $(GLOBALDEPS)
	$(CC) boot.c $(GLOBALFLAGS) $(GTKFLAGS) -c

clean: 
	$(RM) *.o isomaster
	cd bk && $(MAKE) clean
	cd iniparser-2.15 && $(MAKE) clean
	cd po && $(MAKE) clean

# for info about DESTDIR see http://www.gnu.org/prep/standards/html_node/DESTDIR.html

install: isomaster
	$(INSTALL) -d $(DESTDIR)$(BINPATH)
	$(INSTALL) isomaster $(DESTDIR)$(BINPATH)
	$(INSTALL) -d $(DESTDIR)$(ICONPATH)
	$(INSTALL) -m 644 icons/isomaster.png $(DESTDIR)$(ICONPATH)
	$(INSTALL) -m 644 icons/folder-new-tango.png $(DESTDIR)$(ICONPATH)
	$(INSTALL) -m 644 icons/add2-kearone.png $(DESTDIR)$(ICONPATH)
	$(INSTALL) -m 644 icons/extract-kearone.png $(DESTDIR)$(ICONPATH)
	cd po && $(MAKE) install

uninstall: 
	$(RM) -v $(DESTDIR)$(BINPATH)/isomaster
	$(RM) -v $(DESTDIR)$(ICONPATH)/isomaster.png
	$(RM) -v $(DESTDIR)$(ICONPATH)/folder-new-tango.png
	$(RM) -v $(DESTDIR)$(ICONPATH)/add2-kearone.png
	$(RM) -v $(DESTDIR)$(ICONPATH)/extract-kearone.png
	cd po && $(MAKE) uninstall
