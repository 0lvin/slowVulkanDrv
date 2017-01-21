all: libvulkan_slow.so compute

PYTHON2 = python2

# replace usr to something useful if you want not system vulkan includes
vulkan_include_HEADERS = \
	/usr/include/vulkan/vk_platform.h \
	/usr/include/vulkan/vulkan.h

compute: compute.c
	gcc compute.c -o compute -lvulkan

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

slow_wsi.o: slow_wsi.c slow_private.h slow_entrypoints.h
	gcc -fPIC -g -c -Wall slow_wsi.c -Iinclude

slow_wsi_x11.o: slow_wsi_x11.c slow_private.h slow_entrypoints.h
	gcc -fPIC -g -c -Wall slow_wsi_x11.c -Iinclude

libvulkan_slow.so: slow_device.o slow_entrypoints.o slow_formats.o slow_wsi.o slow_wsi_x11.o
	gcc -g -shared -Wl,-soname,libvulkan_slow.so.1 -o $@ slow_device.o slow_formats.o slow_wsi.o slow_wsi_x11.o slow_entrypoints.o -lc

clean:
	rm -f *.o
	rm -f *.so
	rm -f slow_entrypoints.*
	rm -f compute
