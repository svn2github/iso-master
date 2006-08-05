GTKLIBS = `pkg-config --libs gtk+-2.0`
GTKFLAGS = `pkg-config --cflags gtk+-2.0`
GLOBALDEPS = lib

isomaster: isomaster.c lib $(GLOBALDEPS) window.o
	cc isomaster.c window.o $(GTKFLAGS) $(GTKLIBS) -Wall -o isomaster

window.o: window.c window.h $(GLOBALDEPS)
	cc window.c $(GTKFLAGS) -c -Wall

testread2: testread2.c lib
	cc testread2.c bk/bk.a -Wall -o testread2

lib: 
	cd bk && $(MAKE)

clean: 
	rm -f *.o testread2
	cd bk && $(MAKE) clean
