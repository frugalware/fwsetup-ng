CFLAGS ?= -O2
CFLAGS += -std=c99 -Wall -Wextra -ggdb3 -DNEWT -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE $(shell pkg-config --cflags libnewt libparted)
LDFLAGS += $(shell pkg-config --libs libnewt libparted)

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

%.o: %.c fwsetup.h
	$(CC) $(CFLAGS) -c $< -o $@

fwsetup: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(OBJECTS) fwsetup
