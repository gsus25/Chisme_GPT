CFLAGS = -Wall -c

.PHONY: all debug sanitize clean
all: server client

server: server.o
	gcc -o $@ server.o $(DFLAGS) -lpthread

client: client.o
	gcc -o $@ client.o $(DFLAGS) -lpthread

%.o: %.c
	gcc $(CFLAGS) $< $(DFLAGS)

# Compila usando la opción -g para facilitar la depuración con gdb.
debug: DFLAGS = -g
debug: clean all

# Compila habilitando la herramienta AddressSanitizer para
# facilitar la depuración en tiempo de ejecución.
sanitize: DFLAGS = -fsanitize=address,undefined
sanitize: clean all

clean:
	rm -rf server client *.o
