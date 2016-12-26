all: test_drv libvulkan_slow.so

PYTHON2 = python2

# replace usr to something useful if you want not system vulkan includes
vulkan_include_HEADERS = \
	/usr/include/vulkan/vk_platform.h \
	/usr/include/vulkan/vulkan.h

test_drv: test_drv.c
	gcc -g test_drv.c -lvulkan -o test_drv

slow_entrypoints.h: slow_entrypoints_gen.py $(vulkan_include_HEADERS)
	cat $(vulkan_include_HEADERS) | $(PYTHON2) slow_entrypoints_gen.py header > $@

slow_entrypoints.c: slow_entrypoints_gen.py $(vulkan_include_HEADERS)
	cat $(vulkan_include_HEADERS) | $(PYTHON2) slow_entrypoints_gen.py code > $@

slow_entrypoints.o: slow_entrypoints.c slow_entrypoints.h
	gcc -fPIC -g -c -Wall slow_entrypoints.c

head.o: head.c slow_private.h slow_entrypoints.h
	gcc -fPIC -g -c -Wall head.c -Iinclude

libvulkan_slow.so: head.o slow_entrypoints.o
	gcc -shared -Wl,-soname,libvulkan_slow.so.1 -o $@ head.o slow_entrypoints.o -lc

clean:
	rm -f *.o
	rm -f *.so
	rm -f test_drv
	rm -f slow_entrypoints.*
