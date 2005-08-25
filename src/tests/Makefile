testread2: testread2.c testread2.h read7x.o vd.o
	cc testread2.c read7x.o vd.o -Wall -o testread2
read7x.o: read7x.c read7x.h
	cc read7x.c -c
vd.o: vd.c vd.h
	cc vd.c -c
testread: testread.c
	cc testread.c -Wall -o testread
