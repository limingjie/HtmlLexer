# HtmlLexer
A simple HTML Lexer.

## HTML Specification
The Lexer follows [HTML5 Specification](http://www.w3.org/TR/html5/).
- [8 The HTML syntax](http://www.w3.org/TR/html5/syntax.html#syntax)
  - [8.2 Parsing HTML documents](http://www.w3.org/TR/html5/syntax.html#parsing)
    - [8.2.4 Tokenization](http://www.w3.org/TR/html5/syntax.html#tokenization)

## Build
Use makefile in Unix/Linux/MinGW.
```bash
$ make
```

## Unit Test
```bash
$ make test
```

## Sample
- Code: [PrintToken.cpp](https://github.com/limingjie/HtmlLexer/blob/master/PrintToken.cpp)
- Input: [google.html](https://github.com/limingjie/HtmlLexer/blob/master/sample/google.html)
- Output: [google.html.output.txt](https://github.com/limingjie/HtmlLexer/blob/master/sample/google.html.output.txt)

## License
Public Domain
