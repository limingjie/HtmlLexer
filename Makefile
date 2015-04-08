all: HtmlLexer.exe

HtmlLexer.exe: HtmlLexer.cpp
	@echo "==>Compiling HtmlLexer.cpp..."
	g++ -Wall -O2 -std=c++11 -o HtmlLexer.exe HtmlLexer.cpp

test: sample/github.html sample/stackoverflow.html sample/wikipedia.html sample/wikiwand.html
	HtmlLexer.exe sample/github.html        > sample/github.html.output.txt
	HtmlLexer.exe sample/stackoverflow.html > sample/stackoverflow.html.output.txt
	HtmlLexer.exe sample/wikipedia.html     > sample/wikipedia.html.output.txt
	HtmlLexer.exe sample/wikiwand.html      > sample/wikiwand.html.output.txt

clean:
	rm -rf HtmlLexer.exe
	rm -rf sample/*.txt
