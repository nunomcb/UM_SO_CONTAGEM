CC = gcc
FILES = contagem.c client.c
FLAGS = -Wall -Wextra -pedantic -ansi -O2
EXEC = client

build: $(FILES)
	$(CC) $(FLAGS) -o $(EXEC) $(FILES)

debug: FLAGS += -g -DDEBUG
debug: build

final: FLAGS += -O2
final: build

clean:
	rm client
