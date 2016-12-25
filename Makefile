all: test_drv libvulkan_slow.so

test_drv: test_drv.c
	gcc -g test_drv.c -lvulkan -o test_drv

head.o: head.c
	gcc -fPIC -g -c -Wall head.c

libvulkan_slow.so: head.o
	gcc -shared -Wl,-soname,libvulkan_slow.so.1 -o libvulkan_slow.so head.o -lc
    
clean:
	rm -f *.o 
	rm -f *.so 
	rm -f test_drv