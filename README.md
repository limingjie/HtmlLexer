# HtmlLexer
A simple HTML Lexer.

## HTML Specification
The Lexer follows [HTML5 Specification](http://www.w3.org/TR/html5/) on sections below.
- [8.1.2 Elements](http://www.w3.org/TR/html5/syntax.html#elements-0)
  - [Raw text elements](http://www.w3.org/TR/html5/syntax.html#raw-text-elements)
  - [Escapable raw text elements](http://www.w3.org/TR/html5/syntax.html#escapable-raw-text-elements)
  - [8.1.2.1 Start tags](http://www.w3.org/TR/html5/syntax.html#start-tags)
  - [8.1.2.2 End tags](http://www.w3.org/TR/html5/syntax.html#end-tags)
  - [8.1.2.3 Attributes](http://www.w3.org/TR/html5/syntax.html#attributes-0) (See Known Issues #1)

## Known Issues
1. It does not parse "Unquoted attribute value syntax" if it contains `/`.
See
[sample/quora.html](https://github.com/limingjie/HtmlLexer/blob/master/sample/quora.html)
and
[sample/quora.html.output.txt](https://github.com/limingjie/HtmlLexer/blob/master/sample/quora.html.output.txt).
`<script type=text/javascript>`

## License
Public Domain
