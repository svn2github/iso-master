testread2: testread2.c bk/bk.a
	cc testread2.c bk/bk.a -Wall -o testread2

bk/bk.a: 
	cd bk && $(MAKE)

clean: 
	rm -f *.o testread2
	cd bk && $(MAKE) clean
