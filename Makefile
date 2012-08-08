# Setup general C++ flags.
CXXFLAGS ?= -O2
CXXFLAGS := $(CXXFLAGS) -Wall -Wextra -Wno-unused-parameter -ggdb3

# Set library includes.
CXXFLAGS := $(CXXFLAGS) -I/usr/include/yui

# Set linker flags.
LDFLAGS := $(LDFLAGS) -lyui

SOURCES := $(wildcard *.cc)
OBJECTS := $(patsubst %.cc,%.o,$(SOURCES))

setup: $(OBJECTS)
	g++ $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJECTS) setup
