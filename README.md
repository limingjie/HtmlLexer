# HtmlLexer
A HTML Lexical Analyzer.

## HTML Specification
The HtmlLexer follows [HTML5 Specification](http://www.w3.org/TR/html5/).
- [8 The HTML syntax](http://www.w3.org/TR/html5/syntax.html#syntax)
  - [8.2 Parsing HTML documents](http://www.w3.org/TR/html5/syntax.html#parsing)
    - [8.2.4 Tokenization](http://www.w3.org/TR/html5/syntax.html#tokenization)

## Build
Use makefile in Unix/Linux/MinGW.
```bash
$ make clean
$ make
```

## Unit Test
```bash
$ make test
==>Clean Output Files...
rm -rf sample/*.output.txt
==>Unit Test...
html_lexer.exe sample/baidu.html         > sample/baidu.html.output.txt
[Parsing HTML    ] starts...
[Parsing HTML    ] paused.   0m00.016s /       15.600 ticks of 1/1000s
html_lexer.exe sample/facebook.html      > sample/facebook.html.output.txt
[Parsing HTML    ] starts...
[Parsing HTML    ] paused.   0m00.016s /       15.600 ticks of 1/1000s
html_lexer.exe sample/github.html        > sample/github.html.output.txt
[Parsing HTML    ] starts...
[Parsing HTML    ] paused.   0m00.016s /       15.600 ticks of 1/1000s
html_lexer.exe sample/google.html        > sample/google.html.output.txt
[Parsing HTML    ] starts...
[Parsing HTML    ] paused.   0m00.016s /       15.600 ticks of 1/1000s
html_lexer.exe sample/netease.html       > sample/netease.html.output.txt
[Parsing HTML    ] starts...
[Parsing HTML    ] paused.   0m00.031s /       31.200 ticks of 1/1000s
html_lexer.exe sample/quora.html         > sample/quora.html.output.txt
[Parsing HTML    ] starts...
[Parsing HTML    ] paused.   0m00.016s /       15.600 ticks of 1/1000s
html_lexer.exe sample/stackoverflow.html > sample/stackoverflow.html.output.txt
[Parsing HTML    ] starts...
[Parsing HTML    ] paused.   0m00.016s /       15.600 ticks of 1/1000s
html_lexer.exe sample/wikipedia.html     > sample/wikipedia.html.output.txt
[Parsing HTML    ] starts...
[Parsing HTML    ] paused.   0m00.031s /       31.200 ticks of 1/1000s
html_lexer.exe sample/wikiwand.html      > sample/wikiwand.html.output.txt
[Parsing HTML    ] starts...
[Parsing HTML    ] paused.   0m00.031s /       31.200 ticks of 1/1000s
==>Done.
```

## Sample
- Code: [PrintToken.cpp](https://github.com/limingjie/HtmlLexer/blob/master/PrintToken.cpp)
- Input: [google.html](https://github.com/limingjie/HtmlLexer/blob/master/sample/google.html)
- Output: [google.html.output.txt](https://github.com/limingjie/HtmlLexer/blob/master/sample/google.html.output.txt)

## License
Public Domain
