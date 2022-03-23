CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wstrict-prototypes -Wpedantic
SRC     = main.c
BIN     = chunkinfo
RM      = rm -f
CTAGS   = ctags

.default: build

build:
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

debug:
	$(CC) $(CFLAGS) -g $(SRC) -o $(BIN)

debug-asan:
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined $(SRC) -o $(BIN)

clean:
	$(RM) $(BIN) tags

tags:
	$(CTAGS) $(SRC)
