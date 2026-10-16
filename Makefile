CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -march=native -flto -pthread -Wall -Wextra
SRC      := src/main.cpp src/board.cpp src/search.cpp src/uci.cpp

wakachess: $(SRC) src/board.hpp src/search.hpp src/uci.hpp
	$(CXX) $(CXXFLAGS) $(SRC) -o $@

clean:
	rm -f wakachess

.PHONY: clean windows

# Exécutables Windows autonomes (compilation croisée depuis Linux avec MinGW-w64) :
#   bin/WakaChess.exe         interface graphique (souris)
#   bin/WakaChess-console.exe console et protocole UCI
WINCXX   ?= x86_64-w64-mingw32-g++-posix
WINFLAGS := -std=c++17 -O3 -Wall -Wextra -static -s -pthread
MOTEUR   := src/board.cpp src/search.cpp
windows: $(SRC) src/gui_win32.cpp src/board.hpp src/search.hpp src/uci.hpp
	mkdir -p bin
	$(WINCXX) $(WINFLAGS) $(SRC) -o bin/WakaChess-console.exe
	$(WINCXX) $(WINFLAGS) -mwindows src/gui_win32.cpp $(MOTEUR) -o bin/WakaChess.exe
