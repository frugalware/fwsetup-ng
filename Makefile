CXXFLAGS ?= -O2
CXXFLAGS += -Wall -Wextra -Wno-unused-parameter -ggdb3 -DNEWT $(shell pkg-config --cflags libnewt blkid)
LDFLAGS += $(shell pkg-config --libs libnewt blkid)

SOURCES := $(wildcard *.cc)
OBJECTS := $(patsubst %.cc,%.o,$(SOURCES))

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

fwsetup: $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

valgrind: fwsetup
	valgrind --leak-check=full --show-reachable=yes --log-file=valgrind.log ./fwsetup

clean:
	$(RM) $(OBJECTS) fwsetup
