CC = gcc
CFLAGS ?= -std=c99 -O2
CPPFLAGS += -Iinclude -Iextern/include

ZL_ARCHIVE = extern/bin/libzl.a
ECX_DLL = bin/ecx.dll
ECX_SOURCES = $(filter-out src/test.c,$(wildcard src/*.c))
HEADERS = $(wildcard include/ecx/*.h extern/include/zl/*.h src/*.h)

.PHONY: all test clean

all: $(ECX_DLL)

$(ECX_DLL): $(ECX_SOURCES) $(HEADERS) $(ZL_ARCHIVE) | bin
	$(CC) $(CPPFLAGS) $(CFLAGS) -DZL_BUILD_DLL -shared src/ecx.c $(ZL_ARCHIVE) -o $@

bin:
	mkdir -p bin

clean:
	$(RM) $(ECX_DLL)