assembler:	main.o phases.o analyzeline.o dstructs.o errors.h main.h bool.h analyzeline.h dsnodes.h codeandbuild.o 
	gcc -g -ansi -Wall -pedantic main.o phases.o analyzeline.o dstructs.o codeandbuild.o -o assembler
main.o:		main.c phases.o dstructs.o errors.h main.h bool.h dsnodes.h codeandbuild.o
	gcc -c -ansi -Wall -pedantic main.c -o main.o
phases.o:	phases.c analyzeline.o dstructs.o errors.h analyzeline.h bool.h dsnodes.h codeandbuild.o codeandbuild.h
	gcc -c -ansi -Wall -pedantic phases.c -o phases.o
analyzeline.o:	analyzeline.c analyzeline.h bool.h errors.h dsnodes.h
	gcc -c -ansi -Wall -pedantic analyzeline.c -o analyzeline.o
dstructs.o:		dstructs.c dsnodes.h bool.h
	gcc -c -ansi -Wall -pedantic dstructs.c -o dstructs.o
codeandbuild.o:		codeandbuild.c codeandbuild.h analyzeline.o dstructs.o errors.h analyzeline.h bool.h dsnodes.h
	gcc -c -ansi -Wall -pedantic codeandbuild.c -o codeandbuild.o