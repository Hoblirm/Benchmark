all:
	g++ -w -O2 -o run main.c -I./inc
clean:
	rm -f run
