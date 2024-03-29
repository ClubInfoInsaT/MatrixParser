CC=g++
CFLAGS=-Wall -O3 -g
CXXFLAGS=$(CFLAGS)
OBJECTS=parser.o
BINARIES=parser

# Where our library resides. You mostly only need to change the
# RGB_LIB_DISTRIBUTION, this is where the library is checked out.
RGB_LIB_DISTRIBUTION=./lib/lib-led
RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread -pthread

all : $(BINARIES)

.DEFAULT_GOAL := clean all

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

parser : parser.o

% : %.o $(RGB_LIBRARY)
	$(CXX) $< -o $@ $(LDFLAGS)


%.o : %.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $<

%.o : %.c
	$(CC) -I$(RGB_INCDIR) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(BINARIES)
	$(MAKE) -C $(RGB_LIB_DISTRIBUTION) clean

FORCE:
.PHONY: FORCE
