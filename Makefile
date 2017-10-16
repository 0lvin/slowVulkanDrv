all: triangle

triangle: triangle.c
	gcc triangle.c -g -o triangle -pedantic -DVK_USE_PLATFORM_XCB_KHR -lvulkan -lSDL2 -lX11-xcb
