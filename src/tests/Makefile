testread2: testread2.c bkRead7x.o bkAdd.o bkDelete.o bkExtract.o bkRead.o bkPath.o bkMangle.o
	cc testread2.c bkRead7x.o bkAdd.o bkDelete.o bkExtract.o bkRead.o bkPath.o bkMangle.o -Wall -o testread2

clean: 
	rm *.o testread2

bkRead7x.o: bkRead7x.c bkRead7x.h
	cc bkRead7x.c -Wall -c
bkAdd.o: bkAdd.c bkAdd.h bk.h
	cc bkAdd.c -Wall -c
bkDelete.o: bkDelete.c bkDelete.h bk.h
	cc bkDelete.c -Wall -c
bkExtract.o: bkExtract.c bkExtract.h bk.h
	cc bkExtract.c -Wall -c
bkRead.o: bkRead.c bkRead.h bk.h
	cc bkRead.c -Wall -c
bkPath.o: bkPath.c bkPath.h bk.h
	cc bkPath.c -Wall -c
bkMangle.o: bkMangle.c bkMangle.h
	cc bkMangle.c -Wall -c

