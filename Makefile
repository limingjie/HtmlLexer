# Check OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifneq (,$(findstring MINGW,$(uname_S)))
	X=.exe
endif

ifeq ($(OS),Windows_NT)
	X=.exe
endif

# Check CXX flag.
ifeq ($(CXX),clang++)
	CXX=clang++
else
	CXX=g++
endif

CXXFLAGS = -Wall -g -O2 -std=c++0x

all: demo$(X)

html_lexer.o: html_lexer.cpp html_lexer.hpp stopwatch.hpp
	@echo "==>Compiling html_lexer.o..."
	$(CXX) -c $(CXXFLAGS) -o html_lexer.o html_lexer.cpp

demo.o: demo.cpp
	@echo "==>Compiling demo.o..."
	$(CXX) -c $(CXXFLAGS) -o demo.o demo.cpp

demo$(X): html_lexer.o demo.o
	@echo "==>Linking demo$(X)..."
	$(CXX) -o demo$(X) html_lexer.o demo.o

test: demo$(X) cleanoutput \
	sample/baidu.html sample/facebook.html sample/github.html \
	sample/google.html sample/netease.html sample/quora.html \
	sample/stackoverflow.html sample/wikipedia.html sample/wikiwand.html
	@echo "==>Unit Test..."
	./demo$(X) sample/baidu.html         > sample/baidu.html.output.txt
	./demo$(X) sample/facebook.html      > sample/facebook.html.output.txt
	./demo$(X) sample/github.html        > sample/github.html.output.txt
	./demo$(X) sample/google.html        > sample/google.html.output.txt
	./demo$(X) sample/netease.html       > sample/netease.html.output.txt
	./demo$(X) sample/quora.html         > sample/quora.html.output.txt
	./demo$(X) sample/stackoverflow.html > sample/stackoverflow.html.output.txt
	./demo$(X) sample/wikipedia.html     > sample/wikipedia.html.output.txt
	./demo$(X) sample/wikiwand.html      > sample/wikiwand.html.output.txt
	@echo "==>Done."

checkmemoryleak:
	@echo "==>Compiling/Linking with -g..."
	$(CXX) -c -Wall -g -std=c++0x -o html_lexer_d.o html_lexer.cpp
	$(CXX) -c -Wall -g -std=c++0x -o demo_d.o demo.cpp
	$(CXX) -o demo_d$(X) html_lexer_d.o demo_d.o
	@echo "==>Run valgrind..."
	valgrind --leak-check=yes ./demo_d$(X) sample/wikipedia.html > /dev/null
	@echo "==>Cleanup..."
	rm -rf html_lexer_d.o demo_d.o demo_d$(X)
	
cleanoutput:
	@echo "==>Clean Output Files..."
	rm -rf sample/*.output.txt

clean: cleanoutput
	@echo "==>Clean Objects and Executable..."
	rm -rf html_lexer.o demo.o
	rm -rf demo$(X)
