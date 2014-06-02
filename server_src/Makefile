CC = gcc
FILES = estrutura_contagem.c hash_table.c server.c
FLAGS = -Wall -Wextra -ansi
EXEC = server

build: $(FILES)
	$(CC) $(FLAGS) -o $(EXEC) $(FILES)

debug: FLAGS += -g -DDEBUG
debug: build

final: FLAGS += -O2 -Wno-unused-function -Wno-unused-parameter
final: build

clean:
	rm server
