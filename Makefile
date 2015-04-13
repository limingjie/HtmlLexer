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

test: html_lexer.exe cleanoutput sample/baidu.html sample/facebook.html sample/github.html sample/google.html sample/netease.html sample/quora.html sample/stackoverflow.html sample/wikipedia.html sample/wikiwand.html
	@echo "==>Unit Test..."
	html_lexer.exe sample/baidu.html         > sample/baidu.html.output.txt
	@echo "==>Check Output for Errors..."
	@grep "\[Error" sample/*.output.txt
	@echo "==>Done."

cleanoutput:
	@echo "==>Clean Output Files..."
	rm -rf sample/*.output.txt

clean: cleanoutput
	@echo "==>Clean Objects and Executable..."
	rm -rf html_lexer.o PrintToken.o
	rm -rf html_lexer.exe
