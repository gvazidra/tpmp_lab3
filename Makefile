CC = gcc
CFLAGS = -Wall -g
SRC = src/main.c src/db.c src/sqlite3.c
OUT = app

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)

clean:
	del /Q $(OUT).exe 2>nul || rm -f $(OUT)

.PHONY: all clean
