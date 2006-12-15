# other paths are relative to this
ifndef PREFIX
	PREFIX = /usr/local
endif

# where to install the executable
ifndef BINPATH
	BINPATH = $(PREFIX)/bin
endif

# where to install the icons
ifndef ICONPATH
	ICONPATH = $(PREFIX)/share/isomaster/icons
endif

# Used by gettext and po/Makefile (translations)
# To disable installation of .mo files edit or delete the MOFILES 
# variable in po/Makefile. This is safe to do, except obviously
# you won't a have translated ISO Master.
ifndef LOCALEDIR
	export LOCALEDIR = $(PREFIX)/share/locale
endif

# programs used in the Makefiles:
export CC      = gcc
export AR      = ar
export RM      = rm -f
export INSTALL = install

# -DDEBUG and -g only used during development
CFLAGS   = -Wall -pedantic -std=c99 `pkg-config --cflags gtk+-2.0` -g -DDEBUG

# the _FILE_OFFSET_BITS=64 is to enable stat() for large files
CPPFLAGS = -D_FILE_OFFSET_BITS=64

OBJECTS = isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o

all: lib iniparser translations isomaster 

isomaster: $(OBJECTS)
	$(CC) $(OBJECTS) bk/bk.a iniparser-2.15/libiniparser.a $(CFLAGS) $(CPPFLAGS) `pkg-config --libs gtk+-2.0`-o isomaster

# static pattern rule
$(OBJECTS): %.o: %.c bk/bk.h Makefile
	$(CC) $< $(CFLAGS) $(CPPFLAGS) -DICONPATH=\"$(ICONPATH)\" -DLOCALEDIR=\"$(LOCALEDIR)\" -c -o $@

lib:
	cd bk && $(MAKE)

iniparser:
	cd iniparser-2.15 && $(MAKE)

translations:
	cd po && $(MAKE)

clean: 
	cd bk && $(MAKE) clean
	cd iniparser-2.15 && $(MAKE) clean
	cd po && $(MAKE) clean
	$(RM) *.o isomaster

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
	$(RM) $(DESTDIR)$(BINPATH)/isomaster
	$(RM) $(DESTDIR)$(ICONPATH)/isomaster.png
	$(RM) $(DESTDIR)$(ICONPATH)/folder-new-tango.png
	$(RM) $(DESTDIR)$(ICONPATH)/add2-kearone.png
	$(RM) $(DESTDIR)$(ICONPATH)/extract-kearone.png
	cd po && $(MAKE) uninstall
