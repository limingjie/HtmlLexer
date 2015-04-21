all: demo.exe

html_lexer.o: html_lexer.cpp html_lexer.hpp stopwatch.hpp
	@echo "==>Compiling html_lexer.o..."
	g++ -c -o html_lexer.o -Wall -O2 -std=c++0x html_lexer.cpp

demo.o: demo.cpp
	@echo "==>Compiling demo.o..."
	g++ -c -o demo.o -Wall -O2 -std=c++0x demo.cpp

demo.exe: html_lexer.o demo.o
	@echo "==>Linking demo.exe..."
	g++ -Wall -O2 -std=c++0x -o demo.exe html_lexer.o demo.o

test: demo.exe cleanoutput \
	sample/baidu.html sample/facebook.html sample/github.html \
	sample/google.html sample/netease.html sample/quora.html \
	sample/stackoverflow.html sample/wikipedia.html sample/wikiwand.html
	@echo "==>Unit Test..."
	./demo.exe sample/baidu.html         > sample/baidu.html.output.txt
	./demo.exe sample/facebook.html      > sample/facebook.html.output.txt
	./demo.exe sample/github.html        > sample/github.html.output.txt
	./demo.exe sample/google.html        > sample/google.html.output.txt
	./demo.exe sample/netease.html       > sample/netease.html.output.txt
	./demo.exe sample/quora.html         > sample/quora.html.output.txt
	./demo.exe sample/stackoverflow.html > sample/stackoverflow.html.output.txt
	./demo.exe sample/wikipedia.html     > sample/wikipedia.html.output.txt
	./demo.exe sample/wikiwand.html      > sample/wikiwand.html.output.txt
	@echo "==>Done."

checkmemoryleak:
	@echo "==>Compiling/Linking with -g..."
	g++ -c -o html_lexer_g.o -Wall -g -std=c++0x html_lexer.cpp
	g++ -c -o demo_g.o -Wall -g -std=c++0x demo.cpp
	g++ -Wall -g -std=c++0x -o demo_g.exe html_lexer_g.o demo_g.o
	@echo "==>Run valgrind..."
	valgrind --leak-check=yes ./demo_g.exe sample/wikipedia.html > /dev/null
	@echo "==>Cleanup..."
	rm -rf html_lexer_g.o demo_g.o demo_g.exe
	
cleanoutput:
	@echo "==>Clean Output Files..."
	rm -rf sample/*.output.txt

clean: cleanoutput
	@echo "==>Clean Objects and Executable..."
	rm -rf html_lexer.o demo.o
	rm -rf demo.exe
