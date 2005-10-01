testread2: testread2.c read7x.o vd.o bkAdd.o bkDelete.o bkExtract.o bkRead.o bkPath.o
	cc testread2.c read7x.o vd.o bkAdd.o bkDelete.o bkExtract.o bkRead.o bkPath.o -Wall -o testread2

clean: 
	rm *.o testread2

vd.o: vd.c vd.h
	cc vd.c -c

read7x.o: read7x.c read7x.h
	cc read7x.c -c
bkAdd.o: bkAdd.c bkAdd.h bk.h
	cc bkAdd.c -c
bkDelete.o: bkDelete.c bkDelete.h bk.h
	cc bkDelete.c -c
bkExtract.o: bkExtract.c bkExtract.h bk.h
	cc bkExtract.c -c
bkRead.o: bkRead.c bkRead.h bk.h
	cc bkRead.c -c
bkPath.o: bkPath.c bkPath.h bk.h
	cc bkPath.c -c

