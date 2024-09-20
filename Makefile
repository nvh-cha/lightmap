OUT = main.exe
PROD = -mwindows

ENGINE = src/engine.c

SRC = src/main.c $(ENGINE)

all:
	gcc -g -Iinclude -o $(OUT) $(SRC) -Llib -lraylib -lgdi32 -lwinmm
