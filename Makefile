all:
	gcc -w -D__DEBUG -D__LSB test.c interlocked.c threadpool.c
