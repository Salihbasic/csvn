CC="gcc"

test:
	$(CC) -o csvn_t -std=c89 -Wall -Wextra test.c

clean:
	-rm csvn_t 
