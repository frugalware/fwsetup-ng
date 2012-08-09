FLAGS := $(subst -pthread,-lpthread,$(shell pkg-config --cflags --libs glib-2.0 libnewt))

all:
	tcc $(FLAGS) -DNEWT -run setup.c
