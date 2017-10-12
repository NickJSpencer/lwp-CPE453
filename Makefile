CC = gcc
CFLAGS = -Wall -g 
LD = gcc
LDFLAGS = -Wall -g -L.

clean:
	rm -f *.o *.so 

mynums: numbersmain.o liblwp.so
	$(LD) $(LDFLAGS) -o mynums numbersmain.o -llwp

numbersmain.o: numbersmain.c liblwp.so
	$(CC) $(CFLAGS) -g -c -o numbersmain.o numbersmain.c -llwp
    
liblwp.so: lwp.o magic64.o 
	$(CC) $(CFLAGS) -fPIC -shared -o $@ lwp.o magic64.o

lwp.o: lwp.c scheduler.h
	$(CC) $(CFLAGS) -fpic -c -o lwp.o lwp.c

magic64.o: magic64.S
	$(CC) $(CFLAGS) -fpic -c -o magic64.o magic64.S
