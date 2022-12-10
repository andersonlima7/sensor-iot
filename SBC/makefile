all: main

%.o : %.s
	as $(DEBUGFLGS) $(LSTFLGS) $< -o $@

main: main.c
	gcc -o main main.c -lpaho-mqtt3c -lcrypt -lm -lrt -Wall
