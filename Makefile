target:
	clear
	gcc -Wall -g -o main main.c -lpthread

clean:
	rm main

run:
	./main 2 4