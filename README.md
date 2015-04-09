# HtmlLexer
A simple HTML Lexer.

## HTML Specification
The Lexer follows [HTML5 Specification](http://www.w3.org/TR/html5/) on sections below.
- 8.1.2.1 Start tags
- 8.1.2.2 End tags
- 8.1.2.3 Attributes (See Known Issues #1)
- 8.1.2 Elements
  - Raw text elements
  - Escapable raw text elements

## Known Issues
- Can not parse "Unquoted attribute value syntax" if it contains `/`.
See sample/quora.html and sample/quora.html.output.txt.
`<script type=text/javascript>`

## License
Public Domain
