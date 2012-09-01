#g++
CC=g++
CFLAGS=-c -Wall -Dmain=SDL_main -I../lib/LuaJIT/src
LDFLAGS=-L/usr/local/lib -static-libgcc -static-libstdc++ -lmingw32 -lSDLmain -lSDL -lopengl32 -lws2_32 -lwinmm ../lib/LuaJIT/src/libluajit.a
SOURCES=main.cpp IO.cpp LuaState.cpp Asset.cpp AssetStore.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=lualoop

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm *.o *.exe

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $? $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@