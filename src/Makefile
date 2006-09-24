GTKLIBS = `pkg-config --libs gtk+-2.0`
GTKFLAGS = `pkg-config --cflags gtk+-2.0` -g
GLOBALDEPS = bk/bk.h

isomaster: lib iniparser isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o $(GLOBALDEPS)
	cc isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o about.o settings.o boot.o bk/bk.a iniparser-2.15/libiniparser.a $(GTKLIBS) -Wall -o isomaster

lib: 
	cd bk && $(MAKE)

isomaster.o: isomaster.c $(GLOBALDEPS) 
	cc isomaster.c $(GTKFLAGS) -c -Wall

window.o: window.c window.h $(GLOBALDEPS)
	cc window.c $(GTKFLAGS) -c -Wall

browser.o: browser.c $(GLOBALDEPS)
	cc browser.c $(GTKFLAGS) -c -Wall

fsbrowser.o: fsbrowser.c fsbrowser.h $(GLOBALDEPS)
	cc fsbrowser.c $(GTKFLAGS) -c -Wall

isobrowser.o: isobrowser.c isobrowser.h $(GLOBALDEPS)
	cc isobrowser.c $(GTKFLAGS) -c -Wall

error.o: error.c error.h $(GLOBALDEPS)
	cc error.c $(GTKFLAGS) -c -Wall

about.o: about.c about.h $(GLOBALDEPS)
	cc about.c $(GTKFLAGS) -c -Wall

settings.o: settings.c settings.h $(GLOBALDEPS)
	cc settings.c $(GTKFLAGS) -c -Wall

boot.o: boot.c boot.h $(GLOBALDEPS)
	cc boot.c $(GTKFLAGS) -c -Wall

iniparser:
	cd iniparser-2.15 && $(MAKE)

testread2: testread2.c lib
	cc testread2.c bk/bk.a -Wall -o testread2

clean: 
	rm -f *.o isomaster
	cd bk && $(MAKE) clean
	cd iniparser-2.15 && $(MAKE) clean

install: isomaster
	install -d /usr/local/bin/
	install isomaster /usr/local/bin/
	install -d /usr/local/share/isomaster/icons/
	install -m 644 icons/isomaster.svg /usr/local/share/isomaster/icons/
	install -m 644 icons/folder-new.png /usr/local/share/isomaster/icons/

uninstall: 
	rm -f /usr/local/bin/isomaster
	rm -rf /usr/local/share/isomaster/
