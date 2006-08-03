testread2: testread2.c lib
	cc testread2.c bk/bk.a -Wall -o testread2

lib: 
	cd bk && $(MAKE)

clean: 
	rm -f *.o testread2
	cd bk && $(MAKE) clean
