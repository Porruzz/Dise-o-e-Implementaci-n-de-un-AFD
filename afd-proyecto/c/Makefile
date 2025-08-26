CC=gcc
CFLAGS=-O2 -Wall

all: afd

afd: afd.c
	$(CC) $(CFLAGS) -o afd afd.c

run: afd
	./afd ../tests/Conf.txt ../tests/Cadenas.txt

clean:
	rm -f afd
