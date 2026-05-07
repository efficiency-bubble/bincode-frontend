COMPOPT := -std=c++26 -freflection -flto=7 -fuse-linker-plugin -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wno-maybe-uninitialized -Werror -m64 -I"src/include" -I"/usr/include/freetype2" -I$(cppinclude) -fconcepts-diagnostics-depth=10
RELEASEOPT := -O3 -s -DNDEBUG -fcontract-evaluation-semantic=ignore
DEBUGOPT := -O0 -g -fcontract-evaluation-semantic=enforce
FINALOPT := $(COMPOPT) -L$(cpplibs) -lsgl -lSDL3 -lglad -lfreetype -lfontconfig -lstbimage -lharfbuzz -ltbb
LIBNAME := sfe
HEADERS := $(wildcard src/include/$(LIBNAME)/*.hpp) $(wildcard src/include/$(LIBNAME)/*/*.hpp)
SOURCES := $(wildcard src/implementation/*.cpp) $(wildcard src/implementation/*/*.cpp)
OBJECTS := $(patsubst src/implementation/%.cpp,obj/%.o,$(SOURCES))
DEBUG_OBJECTS := $(patsubst src/implementation/%.cpp,obj/%_debug.o,$(SOURCES))
all: lib/$(LIBNAME).a lib/$(LIBNAME)_debug.a
	
bin/%: src/%.cpp lib/$(LIBNAME).a $(cpplibs)/libbbe.a $(HEADERS)
	g++ $< lib/$(LIBNAME).a -o $@ $(RELEASEOPT) $(FINALOPT) -lbbe
bin/%_debug: src/%.cpp lib/$(LIBNAME)_debug.a $(cpplibs)/libbbe_debug.a $(HEADERS)
	g++ $< lib/$(LIBNAME)_debug.a -o $@ $(DEBUGOPT) $(FINALOPT) -lbbe_debug
lib/$(LIBNAME).a: $(OBJECTS)
	ar rcs $@ $(OBJECTS)
lib/$(LIBNAME)_debug.a: $(DEBUG_OBJECTS)
	ar rcs $@ $(DEBUG_OBJECTS)
obj/%.o: src/implementation/%.cpp $(HEADERS)
	g++ -c $< -o $@ $(COMPOPT) $(RELEASEOPT)
obj/%_debug.o: src/implementation/%.cpp $(HEADERS)
	g++ -c $< -o $@ $(COMPOPT) $(DEBUGOPT)
setup:
	mkdir lib
	mkdir bin
	mkdir obj
clean:
	find obj -type f -delete
	find bin -type f -delete
	find lib -type f -delete
.PHONY: setup all clean
	