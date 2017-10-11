CC = gcc
CFLAGS = -Wall -g 
LD = gcc
LDFLAGS = -Wall -g -L.

clean:
	rm -f numbersmain.o *~ TAGS

mynums: numbersmain.o liblwp.so
	$(LD) $(LDFLAGS) -o mynums numbersmain.o -L. -llwp

numbersmain.o: lwp.h 

liblwp.so: lwp.c scheduler.h magic64.S 
	$(CC) $(CFLAGS) -o magic64.o -c magic64.S
	$(CC) $(CFLAGS) -fpic -c lwp.c 
	$(CC) $(CFLAGS) -fpic -shared -o $@ lwp.o magic64.o 
