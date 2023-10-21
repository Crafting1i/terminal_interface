.PHONY: program

program: main.o win.o util.o engine.o
	clang++ -o interface main.o win.o util.o engine.o --std=c++17 -lncurses

main.o: main.cpp
	clang++ -c main.cpp --std=c++17
win.o: win.cpp win.h styles.h
	clang++ -c win.cpp --std=c++17
util.o: util.cpp util.h
	clang++ -c util.cpp --std=c++17
engine.o: engine.cpp engine.h keys.h
	clang++ -c engine.cpp --std=c++17

clean:
	rm -f interface *.o
