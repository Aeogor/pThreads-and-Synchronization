all: raceTest

clean:
	rm raceTest


raceTest: racetest.c
	gcc -std=c99 -pthread -D_SVID_SOURCE -o raceTest racetest.c 


run: raceTest
	./raceTest 31 30 0.1 0.2 42 -noLock
