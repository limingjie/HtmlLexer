all: HtmlLexer.exe

PrintToken.o: PrintToken.cpp
	@echo "==>Compiling PrintToken.o..."
	g++ -c -o PrintToken.o -Wall -O2 -std=c++11 PrintToken.cpp

HtmlLexer.o: HtmlLexer.cpp HtmlLexer.hpp Stopwatch.hpp
	@echo "==>Compiling HtmlLexer.o..."
	g++ -c -o HtmlLexer.o -Wall -O2 -std=c++11 HtmlLexer.cpp

HtmlLexer.exe: HtmlLexer.o PrintToken.o
	@echo "==>Linking HtmlLexer.exe..."
	g++ -Wall -O2 -std=c++11 -o HtmlLexer.exe HtmlLexer.o PrintToken.o

test: HtmlLexer.exe cleanoutput sample/baidu.html sample/facebook.html sample/github.html sample/google.html sample/netease.html sample/quora.html sample/stackoverflow.html sample/wikipedia.html sample/wikiwand.html
	@echo "==>Unit Test..."
	HtmlLexer.exe sample/baidu.html         > sample/baidu.html.output.txt
	HtmlLexer.exe sample/facebook.html      > sample/facebook.html.output.txt
	HtmlLexer.exe sample/github.html        > sample/github.html.output.txt
	HtmlLexer.exe sample/google.html        > sample/google.html.output.txt
	HtmlLexer.exe sample/netease.html       > sample/netease.html.output.txt
	HtmlLexer.exe sample/quora.html         > sample/quora.html.output.txt
	HtmlLexer.exe sample/stackoverflow.html > sample/stackoverflow.html.output.txt
	HtmlLexer.exe sample/wikipedia.html     > sample/wikipedia.html.output.txt
	HtmlLexer.exe sample/wikiwand.html      > sample/wikiwand.html.output.txt
	@echo "==>Check Output for Errors..."
	@grep "\[Error" sample/*.output.txt
	@echo "==>Done."

cleanoutput:
	@echo "==>Clean Output Files..."
	rm -rf sample/*.output.txt

clean: cleanoutput
	@echo "==>Clean Objects and Executable..."
	rm -rf HtmlLexer.o PrintToken.o
	rm -rf HtmlLexer.exe
