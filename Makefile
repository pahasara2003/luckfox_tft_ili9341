CXX ?= g++
CXXFLAGS ?= -O2 -std=c++11 -Wall -Wextra

all: example

examples: build/basic build/sprite build/photo_demo

build/%: examples/%.cpp examples/board_config.h ILI9341.cpp ILI9341.h Sprite.cpp Sprite.h font5x7.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) -I. -o $@ $< ILI9341.cpp Sprite.cpp

example: example.cpp ILI9341.cpp ILI9341.h Sprite.cpp Sprite.h font5x7.h
	$(CXX) $(CXXFLAGS) -o $@ example.cpp ILI9341.cpp Sprite.cpp

bench: bench.cpp ILI9341.cpp ILI9341.h font5x7.h
	$(CXX) $(CXXFLAGS) -o $@ bench.cpp ILI9341.cpp

test: test.cpp ILI9341.cpp ILI9341.h Sprite.cpp Sprite.h font5x7.h
	$(CXX) $(CXXFLAGS) -o $@ test.cpp ILI9341.cpp Sprite.cpp
	./test

clean:
	rm -f example test bench
	rm -rf build

.PHONY: all clean test examples
