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
	gcc -fPIC -g -c -Wall slow_entrypoints.c -Iinclude

slow_device.o: slow_device.c slow_private.h slow_entrypoints.h
	gcc -fPIC -g -c -Wall slow_device.c -Iinclude

slow_formats.o: slow_formats.c slow_private.h slow_entrypoints.h
	gcc -fPIC -g -c -Wall slow_formats.c -Iinclude

libvulkan_slow.so: slow_device.o slow_entrypoints.o slow_formats.o
	gcc -shared -Wl,-soname,libvulkan_slow.so.1 -o $@ slow_device.o slow_entrypoints.o slow_formats.o -lc

clean:
	rm -f *.o
	rm -f *.so
	rm -f test_drv
	rm -f slow_entrypoints.*
