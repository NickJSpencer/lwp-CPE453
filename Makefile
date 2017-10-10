CC = gcc
CFLAGS = -Wall -g -I .
LD = gcc
LDFLAGS = -Wall -g -L.

clean:
	rm -f numbersmain.o *~ TAGS

mynums: numbersmain.o libLWP.a
	$(LD) $(LDFLAGS) -o mynums numbersmain.o -L. -lLWP

numbersmain.o: lwp.h 

libLWP.a: lwp.c scheduler.h
	gcc -c lwp.c
	ar r libLWP.a lwp.o
	rm lwp.o
