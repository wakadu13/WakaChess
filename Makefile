CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -march=native -flto -pthread -Wall -Wextra
SRC      := src/main.cpp src/board.cpp src/search.cpp src/uci.cpp

wakachess: $(SRC) src/board.hpp src/search.hpp src/uci.hpp
	$(CXX) $(CXXFLAGS) $(SRC) -o $@

clean:
	rm -f wakachess

.PHONY: clean windows

# Exécutable Windows autonome (compilation croisée depuis Linux avec MinGW-w64)
WINCXX ?= x86_64-w64-mingw32-g++-posix
windows: $(SRC) src/board.hpp src/search.hpp src/uci.hpp
	mkdir -p bin
	$(WINCXX) -std=c++17 -O3 -Wall -Wextra -static -s -pthread $(SRC) -o bin/WakaChess.exe
