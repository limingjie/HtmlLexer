# HtmlLexer

[![Build Status](https://travis-ci.org/limingjie/HtmlLexer.svg?branch=master)](https://travis-ci.org/limingjie/HtmlLexer)

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

size_t start, end, list_start, list_end, start_pos, end_pos;

// find <ul> tag that has classes "header-nav" and "right" from beginning.
start = lexer.find_tag_by_class_names("ul","header-nav right", 0);

// check result
if (start == html_lexer::npos)
{
    // failed
}

// find the <ul> tag's end tag </ul>.
end = lexer.find_matching_tag(start);

// find </li> tag after <ul> tag which we found.
list_end = lexer.find_tag_by_name("li", false, start + 1);

// find the </li> tag's start tag <li>
list_start = lexer.find_matching_tag(list_end);

// get the original position in html [start_pos, end_pos)
start_pos = lexer.get_token(list_start)->get_start_position();
end_pos = lexer.get_token(list_end)->get_end_position();

// out the original html content <li>...</li>
std::cout << html.substr(start_pos, end_pos - start_pos) << std::endl;
```

## Build

Use makefile in Unix/Linux/MinGW.

```bash
$ make clean
$ make
==>Compiling html_lexer.o...
g++ -c -Wall -g -O2 -std=c++0x -o html_lexer.o html_lexer.cpp
==>Compiling demo.o...
g++ -c -Wall -g -O2 -std=c++0x -o demo.o demo.cpp
==>Linking demo.exe...
g++ -o demo.exe html_lexer.o demo.o
```

## Unit Test

```bash
$ make test
==>Clean Output Files...
rm -rf sample/*.output.txt
==>Unit Test...
./demo sample/baidu.html         > sample/baidu.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.003s /        2.688 x 1/1000s
./demo sample/facebook.html      > sample/facebook.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.001s /        1.340 x 1/1000s
./demo sample/github.html        > sample/github.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.003s /        3.129 x 1/1000s
./demo sample/google.html        > sample/google.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.001s /        1.268 x 1/1000s
./demo sample/netease.html       > sample/netease.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.017s /       16.697 x 1/1000s
./demo sample/quora.html         > sample/quora.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.006s /        6.497 x 1/1000s
./demo sample/stackoverflow.html > sample/stackoverflow.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.010s /        9.523 x 1/1000s
./demo sample/wikipedia.html     > sample/wikipedia.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.011s /       11.257 x 1/1000s
./demo sample/wikiwand.html      > sample/wikiwand.html.output.txt
[Tokenize HTML   ] starts...
[Tokenize HTML   ] done.     0m00.014s /       14.012 x 1/1000s
==>Done.
```

## Sample

- Code: [demo.cpp](https://github.com/limingjie/HtmlLexer/blob/master/demo.cpp)
- Input: [google.html](https://github.com/limingjie/HtmlLexer/blob/master/sample/google.html)
- Output: [google.html.output.txt](https://github.com/limingjie/HtmlLexer/blob/master/sample/google.html.output.txt)

## Known Issue

The library only supports ASCII or UTF-8 encoded HTML.

## License

Public Domain
