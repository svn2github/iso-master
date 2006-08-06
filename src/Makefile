GTKLIBS = `pkg-config --libs gtk+-2.0`
GTKFLAGS = `pkg-config --cflags gtk+-2.0`
GLOBALDEPS = 

isomaster: isomaster.o window.o fsbrowser.o error.o lib $(GLOBALDEPS)
	cc isomaster.o window.o fsbrowser.o error.o $(GTKFLAGS) $(GTKLIBS) -Wall -o isomaster

isomaster.o: isomaster.c $(GLOBALDEPS) 
	cc isomaster.c $(GTKFLAGS) -c -Wall

window.o: window.c window.h $(GLOBALDEPS)
	cc window.c $(GTKFLAGS) -c -Wall

fsbrowser.o: fsbrowser.c fsbrowser.h $(GLOBALDEPS)
	cc fsbrowser.c $(GTKFLAGS) -c -Wall

error.o: error.c error.h $(GLOBALDEPS)
	cc error.c $(GTKFLAGS) -c -Wall

testread2: testread2.c lib
	cc testread2.c bk/bk.a -Wall -o testread2

lib: 
	cd bk && $(MAKE)

clean: 
	rm -f *.o testread2
	cd bk && $(MAKE) clean
