all: libvulkan_cpu.so compute

PYTHON2 = python2

# replace usr to something useful if you want not system vulkan includes
vulkan_include_HEADERS = \
	/usr/include/vulkan/vk_platform.h \
	/usr/include/vulkan/vulkan.h

compute: compute.c
	gcc -fPIC -g compute.c -o compute -lvulkan

cpu_entrypoints.h: cpu_entrypoints_gen.py $(vulkan_include_HEADERS)
	cat $(vulkan_include_HEADERS) | $(PYTHON2) cpu_entrypoints_gen.py header > $@

cpu_entrypoints.c: cpu_entrypoints_gen.py $(vulkan_include_HEADERS)
	cat $(vulkan_include_HEADERS) | $(PYTHON2) cpu_entrypoints_gen.py code > $@

cpu_entrypoints.o: cpu_entrypoints.c cpu_entrypoints.h
	gcc -fPIC -g -c -Wall cpu_entrypoints.c -Iinclude

cpu_device.o: cpu_device.c cpu_private.h cpu_entrypoints.h
	gcc -fPIC -g -c -Wall cpu_device.c -Iinclude

cpu_formats.o: cpu_formats.c cpu_private.h cpu_entrypoints.h
	gcc -fPIC -g -c -Wall cpu_formats.c -Iinclude

cpu_wsi.o: cpu_wsi.c cpu_private.h cpu_entrypoints.h
	gcc -fPIC -g -c -Wall cpu_wsi.c -Iinclude

cpu_wsi_x11.o: cpu_wsi_x11.c cpu_private.h cpu_entrypoints.h
	gcc -fPIC -g -c -Wall cpu_wsi_x11.c -Iinclude

cpu_pipeline.o: cpu_pipeline.c cpu_private.h cpu_entrypoints.h
	gcc -fPIC -g -c -Wall cpu_pipeline.c -Iinclude

libvulkan_cpu.so: cpu_device.o cpu_entrypoints.o cpu_formats.o cpu_wsi.o cpu_wsi_x11.o cpu_pipeline.o
	gcc -g -shared -Wl,-soname,libvulkan_cpu.so.1 -o $@ cpu_device.o cpu_pipeline.o cpu_formats.o cpu_wsi.o cpu_wsi_x11.o cpu_entrypoints.o -lc

clean:
	rm -f *.o
	rm -f *.so
	rm -f cpu_entrypoints.*
	rm -f compute
