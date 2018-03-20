obj-m := xbmd.o

path := $(shell uname -r)

all:
	gtk-builder-convert xbmd_app.glade xbmd_app.xml
	make -C /lib/modules/$(path)/build M=/root/xbmd  modules
	g++ -Wall -g -o xbmd_app cfg.cpp bmd.cpp xbmd_ep.cpp xbmd_main.cpp `pkg-config --cflags --libs gtk+-2.0 --libs libglade-2.0` -export-dynamic
clean:
	make -C /lib/modules/$(path)/build M=/root/xbmd clean
