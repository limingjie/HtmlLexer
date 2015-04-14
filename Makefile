all: html_lexer.exe

PrintToken.o: PrintToken.cpp
	@echo "==>Compiling PrintToken.o..."
	g++ -c -o PrintToken.o -Wall -O2 -std=c++11 PrintToken.cpp

html_lexer.o: html_lexer.cpp html_lexer.hpp Stopwatch.hpp
	@echo "==>Compiling html_lexer.o..."
	g++ -c -o html_lexer.o -Wall -O2 -std=c++11 html_lexer.cpp

html_lexer.exe: html_lexer.o PrintToken.o
	@echo "==>Linking html_lexer.exe..."
	g++ -Wall -O2 -std=c++11 -o html_lexer.exe html_lexer.o PrintToken.o

test: html_lexer.exe cleanoutput \
	sample/baidu.html sample/facebook.html sample/github.html \
	sample/google.html sample/netease.html sample/quora.html \
	sample/stackoverflow.html sample/wikipedia.html sample/wikiwand.html
	@echo "==>Unit Test..."
	html_lexer.exe sample/baidu.html         > sample/baidu.html.output.txt
	html_lexer.exe sample/facebook.html      > sample/facebook.html.output.txt
	html_lexer.exe sample/github.html        > sample/github.html.output.txt
	html_lexer.exe sample/google.html        > sample/google.html.output.txt
	html_lexer.exe sample/netease.html       > sample/netease.html.output.txt
	html_lexer.exe sample/quora.html         > sample/quora.html.output.txt
	html_lexer.exe sample/stackoverflow.html > sample/stackoverflow.html.output.txt
	html_lexer.exe sample/wikipedia.html     > sample/wikipedia.html.output.txt
	html_lexer.exe sample/wikiwand.html      > sample/wikiwand.html.output.txt
	@echo "==>Done."

cleanoutput:
	@echo "==>Clean Output Files..."
	rm -rf sample/*.output.txt

clean: cleanoutput
	@echo "==>Clean Objects and Executable..."
	rm -rf html_lexer.o PrintToken.o
	rm -rf html_lexer.exe
