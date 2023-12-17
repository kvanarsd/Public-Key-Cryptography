CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic $(shell pkg-config --cflags gmp)
LFLAGS = $(shell pkg-config --libs gmp)
EXEC = keygen encrypt decrypt
OBJECTS = ss.o randstate.o numtheory.o

all: $(EXEC)

keygen: keygen.o $(OBJECTS)
	$(CC) -o $@ $^ $(LFLAGS)

encrypt: encrypt.o $(OBJECTS)
	$(CC) -o $@ $^ $(LFLAGS)

decrypt: decrypt.o $(OBJECTS)
	$(CC) -o $@ $^ $(LFLAGS)

ss.o: ss.c
	$(CC) $(CFLAGS) -c ss.c
	
randstate.o: randstate.c
	$(CC) $(CFLAGS) -c randstate.c

numtheory.o: numtheory.c
	$(CC) $(CFLAGS) -c numtheory.c

clean:
	rm -f $(EXEC) $(OBJECTS) decrypt.o keygen.o encrypt.o
format:
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build --use-cc=$(CC) make
