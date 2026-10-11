CC ?= gcc
veyra-pcforge: pcforge.c
	$(CC) -O2 -Wall -o veyra-pcforge pcforge.c
clean:
	rm -f veyra-pcforge
