target:
	clear
	gcc -g -o main -Wall -pthread main.c

clean:
	rm main

run:
	./main 2 4