all:
	gcc -w -D__DEBUG -D__LSB -D__LOG -pthread test.c interlocked.c threadpool.c
