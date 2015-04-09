all: HtmlLexer.exe

HtmlLexer.exe: HtmlLexer.cpp
	@echo "==>Compiling HtmlLexer.cpp..."
	g++ -Wall -O2 -std=c++11 -o HtmlLexer.exe HtmlLexer.cpp

test: HtmlLexer.exe sample/baidu.html sample/facebook.html sample/github.html sample/google.html sample/netease.html sample/quora.html sample/stackoverflow.html sample/wikipedia.html sample/wikiwand.html
	HtmlLexer.exe sample/baidu.html         > sample/baidu.html.output.txt
	HtmlLexer.exe sample/facebook.html      > sample/facebook.html.output.txt
	HtmlLexer.exe sample/github.html        > sample/github.html.output.txt
	HtmlLexer.exe sample/google.html        > sample/google.html.output.txt
	HtmlLexer.exe sample/netease.html       > sample/netease.html.output.txt
	HtmlLexer.exe sample/quora.html         > sample/quora.html.output.txt
	HtmlLexer.exe sample/stackoverflow.html > sample/stackoverflow.html.output.txt
	HtmlLexer.exe sample/wikipedia.html     > sample/wikipedia.html.output.txt
	HtmlLexer.exe sample/wikiwand.html      > sample/wikiwand.html.output.txt

clean:
	rm -rf HtmlLexer.exe
	rm -rf sample/*.txt
