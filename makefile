all:
	g++ -w -O2 -o run main.c -I./inc -lrt
clean:
	rm -f run
