
CC=gcc

ARFLAGS   = rv

CPPFLAGS += -I../ext
CFLAGS   += -g -Wall

LDDIRS   += -L../ext
LDLIBS   += -lm
LDFLAGS  += -g

SOURCES = $(wildcard *.c) ../ext/bitpack.c
OBJECTS = $(SOURCES:%.c=%.o)

run_tests: test_driver
	./test_driver

test_driver: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDDIRS) $(LDLIBS)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -f test_driver *.o core

