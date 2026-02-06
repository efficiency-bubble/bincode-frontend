COMPOPT := -std=c++26 -flto=7 -fuse-linker-plugin -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -m64 -O0 -g -I"src/include" -I"/usr/include/freetype2" -I$(cppinclude) -fsanitize=address
FINALOPT := $(COMPOPT) -L$(cpplibs) -lsgl -lSDL3 -lglad -lfreetype -lfontconfig -lstbimage -lharfbuzz -lbbe
LIBNAME := sfe
HEADERS := $(wildcard src/include/$(LIBNAME)/*.hpp) $(wildcard src/include/$(LIBNAME)/*/*.hpp)
SOURCES := $(wildcard src/implementation/*.cpp) $(wildcard src/implementation/*/*.cpp)
OBJECTS := $(patsubst src/implementation/%.cpp,obj/%.o,$(SOURCES))
bin/%: src/%.cpp lib/$(LIBNAME).a
	g++ $< lib/$(LIBNAME).a -o $@ $(FINALOPT)
lib/$(LIBNAME).a: $(OBJECTS)
	ar rcs $@ $(OBJECTS)
obj/%.o: src/implementation/%.cpp $(HEADERS)
	g++ -c $< -o $@ $(COMPOPT)
setup:
	mkdir lib
	mkdir bin
	mkdir obj
.PHONY: setup
	