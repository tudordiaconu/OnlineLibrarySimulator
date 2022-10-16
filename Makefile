client: client.c requests.c helpers.c buffer.c
	gcc -g -o client client.c requests.c helpers.c buffer.c -Wall -Wextra

run: client
	./client

clean:
	rm -f *.o client
