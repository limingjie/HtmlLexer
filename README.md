# HtmlLexer
A HTML Lexical Analyzer.

## HTML Specification
The library follows [HTML5 Specification](http://www.w3.org/TR/html5/).
- [8 The HTML syntax](http://www.w3.org/TR/html5/syntax.html#syntax)
  - [8.2 Parsing HTML documents](http://www.w3.org/TR/html5/syntax.html#parsing)
    - [8.2.4 Tokenization](http://www.w3.org/TR/html5/syntax.html#tokenization)

## Interface
```c++
// define lexer and tokenize html.
html_lexer lexer(html);

// find <ul> tag that has classes "header-nav" and "right" from beginning.
size_t start = lexer.find_tag_by_class_names("ul","header-nav right", 0);

// find the <ul> tag's end tag </ul>.
size_t end = lexer.find_matching_tag(start);

// find </li> tag after <ul> tag which we found.
size_t list_end = find_tag_by_name("li", false, start + 1);

// find the </li> tag's start tag <li>
size_t list_start = lexer.find_matching_tag(list_end);

// get the original position in html [start_pos, end_pos)
size_t start_pos lexer.get_token(list_start)->get_start_position();
size_t end_pos lexer.get_token(list_end)->get_end_position();
```

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
demo.exe sample/baidu.html         > sample/baidu.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.006s /        6.001 x 1/1000s
demo.exe sample/facebook.html      > sample/facebook.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.002s /        2.001 x 1/1000s
demo.exe sample/github.html        > sample/github.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.004s /        4.000 x 1/1000s
demo.exe sample/google.html        > sample/google.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.002s /        2.000 x 1/1000s
demo.exe sample/netease.html       > sample/netease.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.019s /       19.001 x 1/1000s
demo.exe sample/quora.html         > sample/quora.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.007s /        7.000 x 1/1000s
demo.exe sample/stackoverflow.html > sample/stackoverflow.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.015s /       15.001 x 1/1000s
demo.exe sample/wikipedia.html     > sample/wikipedia.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.016s /       16.001 x 1/1000s
demo.exe sample/wikiwand.html      > sample/wikiwand.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.019s /       19.001 x 1/1000s
==>Done.
```

## Sample
- Code: [demo.cpp](https://github.com/limingjie/HtmlLexer/blob/master/demo.cpp)
- Input: [google.html](https://github.com/limingjie/HtmlLexer/blob/master/sample/google.html)
- Output: [google.html.output.txt](https://github.com/limingjie/HtmlLexer/blob/master/sample/google.html.output.txt)

## Known Issue
The library only process ASCII or UTF-8 encoded HTML.

## License
Public Domain
