CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Deteksi OS
ifeq ($(OS),Windows_NT)
    LIBS = -lws2_32
    EXT = .exe
else
    LIBS = -pthread
    EXT =
endif

all: server$(EXT) client$(EXT)

server$(EXT): server.cpp MenuItem.h MenuLinkedList.h Algorithms.h Order.h JsonHelper.h
	$(CXX) $(CXXFLAGS) -o server$(EXT) server.cpp $(LIBS)

client$(EXT): client.cpp MenuItem.h JsonHelper.h
	$(CXX) $(CXXFLAGS) -o client$(EXT) client.cpp $(LIBS)

clean:
	rm -f server server.exe client client.exe

.PHONY: all clean
