# cut is necessary for Cygwin
PLATFORM_OS := $(shell uname | cut -d_ -f1)

all: bin2png png2bin functionalTest

clean:
	@rm -rf *.o bin2png png2bin test

CFLAGS = -D_XOPEN_SOURCE=600 -std=c99 -Wall -Wextra $(ADD_FLAGS)
LDFLAGS = -lpng

ifeq ($(PLATFORM_OS), Linux)
	LDFLAGS += -lm --coverage # required for sqrt()
endif

common.o: common.h
	echo $(PLATFORM_OS)
	clang -o $@ -c common.c $(CFLAGS)

imgify.o: imgify.h
	clang -o $@ -c imgify.c $(CFLAGS)

bin2png: common.o imgify.o
	clang -o $@ bin2png.c $^ $(CFLAGS) $(LDFLAGS)

png2bin: common.o imgify.o
	clang -o $@ png2bin.c $^ $(CFLAGS) $(LDFLAGS)

functionalTest: common.o imgify.o
	clang -o $@ functionalTest.c $^ -fprofile-arcs -ftest-coverage -O0 $(CFLAGS) $(LDFLAGS)
