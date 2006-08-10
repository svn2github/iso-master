GTKLIBS = `pkg-config --libs gtk+-2.0`
GTKFLAGS = `pkg-config --cflags gtk+-2.0`
GLOBALDEPS = 

isomaster: lib isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o $(GLOBALDEPS)
	cc isomaster.o window.o browser.o fsbrowser.o isobrowser.o error.o bk/bk.a $(GTKLIBS) -Wall -g -o isomaster

isomaster.o: isomaster.c $(GLOBALDEPS) 
	cc isomaster.c $(GTKFLAGS) -c -g -Wall

window.o: window.c window.h $(GLOBALDEPS)
	cc window.c $(GTKFLAGS) -c -g -Wall

browser.o: browser.c $(GLOBALDEPS)
	cc browser.c $(GTKFLAGS) -c -g -Wall

fsbrowser.o: fsbrowser.c fsbrowser.h $(GLOBALDEPS)
	cc fsbrowser.c $(GTKFLAGS) -c -g -Wall

isobrowser.o: isobrowser.c isobrowser.h $(GLOBALDEPS)
	cc isobrowser.c $(GTKFLAGS) -c -g -Wall

error.o: error.c error.h $(GLOBALDEPS)
	cc error.c $(GTKFLAGS) -c -g -Wall

testread2: testread2.c lib
	cc testread2.c bk/bk.a -Wall -g -o testread2

lib: 
	cd bk && $(MAKE)

clean: 
	rm -f *.o testread2
	cd bk && $(MAKE) clean
