np=31

run: compile
	mpirun --oversubscribe -np $(np) bin/main

compile:
	@mkdir -p bin && rm -f bin/*
	mpicxx -O2 -pedantic -Wall barrier.cpp -o bin/barrier.o -c
	mpicxx -O2 -pedantic -Wall main.cpp bin/barrier.o -o bin/main
	
clean:
	@rm -rf bin

install:
	sudo apt-get install libopenmpi-dev openmpi-bin
