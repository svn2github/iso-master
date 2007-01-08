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
# To disable installation of some .mo files edit or delete the MOFILES 
# variable in po/Makefile. This is safe to do.
ifndef LOCALEDIR
	export LOCALEDIR = $(PREFIX)/share/locale
endif

# where to install the man page
ifndef MYMANPATH
	MYMANPATH = $(PREFIX)/man/man1
endif

# To disable i18n completely, uncomment the following line 
# or define WITHOUT_NLS somewhere else.
# This option is desired in the FreeBSD ports guidelines.
#WITHOUT_NLS = 1

# programs used in the Makefiles:
export CC      = gcc
export AR      = ar
export RM      = rm -f
export INSTALL = install

# -DDEBUG and -g only used during development
CFLAGS   = -Wall -pedantic -std=c99 `pkg-config --cflags gtk+-2.0` -DICONPATH=\"$(ICONPATH)\" -DLOCALEDIR=\"$(LOCALEDIR)\"
ifndef WITHOUT_NLS
	CFLAGS += -DENABLE_NLS
endif

# the _FILE_OFFSET_BITS=64 is to enable stat() for large files
CPPFLAGS = -D_FILE_OFFSET_BITS=64

OBJECTS = isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o

all: lib iniparser translations isomaster man

isomaster: $(OBJECTS) bk/bk.a iniparser-2.15/libiniparser.a
	$(CC) $(OBJECTS) bk/bk.a iniparser-2.15/libiniparser.a $(CFLAGS) $(CPPFLAGS) `pkg-config --libs gtk+-2.0`-o isomaster

# static pattern rule
$(OBJECTS): %.o: %.c bk/bk.h Makefile
	$(CC) $< $(CFLAGS) $(CPPFLAGS) -c -o $@

lib:
	cd bk && $(MAKE)

iniparser:
	cd iniparser-2.15 && $(MAKE)

translations:
ifndef WITHOUT_NLS
	cd po && $(MAKE)
endif

man:
	gzip -c isomaster.1 > isomaster.1.gz

clean: 
	cd bk && $(MAKE) clean
	cd iniparser-2.15 && $(MAKE) clean
ifndef WITHOUT_NLS
	cd po && $(MAKE) clean
endif
	$(RM) *.o isomaster

# for info about DESTDIR see http://www.gnu.org/prep/standards/html_node/DESTDIR.html

install: isomaster
	$(INSTALL) -d $(DESTDIR)$(BINPATH)
	$(INSTALL) isomaster $(DESTDIR)$(BINPATH)
	$(INSTALL) -d $(DESTDIR)$(ICONPATH)
	$(INSTALL) -m 644 icons/isomaster.png $(DESTDIR)$(ICONPATH)
	$(INSTALL) -m 644 icons/folder-new-tango.png $(DESTDIR)$(ICONPATH)
	$(INSTALL) -m 644 icons/add2-kearone.png $(DESTDIR)$(ICONPATH)
	$(INSTALL) -m 644 icons/extract2-kearone.png $(DESTDIR)$(ICONPATH)
ifndef WITHOUT_NLS
	cd po && $(MAKE) install
endif
	$(INSTALL) -d $(DESTDIR)$(MYMANPATH)
	$(INSTALL) -m 644 isomaster.1.gz $(DESTDIR)$(MYMANPATH)

uninstall: 
	$(RM) $(DESTDIR)$(BINPATH)/isomaster
	$(RM) $(DESTDIR)$(ICONPATH)/isomaster.png
	$(RM) $(DESTDIR)$(ICONPATH)/folder-new-tango.png
	$(RM) $(DESTDIR)$(ICONPATH)/add2-kearone.png
	$(RM) $(DESTDIR)$(ICONPATH)/extract-kearone.png
ifndef WITHOUT_NLS
	cd po && $(MAKE) uninstall
endif

