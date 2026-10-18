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

# Exécutables Linux autonomes (liés statiquement) : bin/linux-x64 et bin/linux-arm64 (Raspberry Pi…)
LINFLAGS := -std=c++17 -O3 -Wall -Wextra -static -s -pthread
linux: $(SRC) src/board.hpp src/search.hpp src/uci.hpp
	mkdir -p bin/linux-x64 bin/linux-arm64
	g++ $(LINFLAGS) $(SRC) -o bin/linux-x64/wakachess
	aarch64-linux-gnu-g++ $(LINFLAGS) $(SRC) -o bin/linux-arm64/wakachess

# Version web (WebAssembly) dans docs/ : nécessite Emscripten (em++)
WEB_FONCTIONS := _wc_nouvelle,_wc_fen,_wc_jouer,_wc_annuler,_wc_legaux,_wc_plateau,_wc_trait,_wc_echec,_wc_case_roi,_wc_etat,_wc_chercher,_wc_vider_tt
web: src/web_api.cpp $(MOTEUR) src/board.hpp src/search.hpp
	em++ -std=c++17 -O3 -Wall -Wextra src/web_api.cpp $(MOTEUR) -o docs/wakachess.js \
		-sMODULARIZE=1 -sEXPORT_NAME=WakaChessModule -sENVIRONMENT=web,worker,node -sSINGLE_FILE=1 \
		-sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=33554432 \
		-sEXPORTED_FUNCTIONS=$(WEB_FONCTIONS) -sEXPORTED_RUNTIME_METHODS=cwrap

.PHONY: linux web
