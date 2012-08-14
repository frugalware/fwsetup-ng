CFLAGS ?= -O2
CFLAGS += -std=c99 -Wall -Wextra -ggdb3 -DNEWT -D_POSIX_C_SOURCE=200809L -D_BSD_SOURCE -D_XOPEN_SOURCE $(shell pkg-config --cflags libnewt blkid)
LDFLAGS += $(shell pkg-config --libs libnewt blkid)

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

%.o: %.c fwsetup.h
	$(CC) $(CFLAGS) -c $< -o $@

fwsetup: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

valgrind: fwsetup
	valgrind --leak-check=full --show-reachable=yes --log-file=valgrind.log ./fwsetup

clean:
	$(RM) $(OBJECTS) fwsetup
