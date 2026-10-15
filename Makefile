CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -march=native -flto -Wall -Wextra
SRC      := src/main.cpp src/board.cpp src/search.cpp

wakachess: $(SRC) src/board.hpp src/search.hpp
	$(CXX) $(CXXFLAGS) $(SRC) -o $@

clean:
	rm -f wakachess

.PHONY: clean
