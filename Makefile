simple: goof.c goof.h frame.c frame.h simple.c
	gcc -g `pkg-config --cflags --libs glib-2.0 gobject-2.0` goof.c\
        frame.c simple.c \
	    -o simple
