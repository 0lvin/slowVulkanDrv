all: compute

compute: compute.c
	gcc -fPIC -g compute.c -o compute -lvulkan

clean:
	rm -f *.o
	rm -f *.so
	rm -f compute
