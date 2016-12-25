test_drv: test_drv.c
	gcc -g test_drv.c -lvulkan -o test_drv

all: test_drv

clean:
	rm -f *.o 
	rm -f test_drv