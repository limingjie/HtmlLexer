//
// HTML Lexer
// A HTML Lexer which follows HTML5 specification
//
// Author - Mingjie Li
// Email  - limingjie@outlook.com
//          limingjie0719@gmail.com
// Date   - Apr 12, 2015
// Github - https://github.com/limingjie/HtmlLexer
//
#ifndef __HTML_LEXER__
#define __HTML_LEXER__

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <cctype> // tolower(), isupper(), islower()

class html_lexer;

// abstract base class for html tokens
class html_token
{
    friend class html_lexer;

public:
    enum token_type
    {
        token_null,
        token_start_tag,
        token_end_tag,
        token_comment,
        token_bogus_comment,
        token_text,
        token_raw_text
    };

private:
    // the position [start, end) of the token in original html
    size_t _start;
    size_t _end;

    // the type of token
    token_type _type;

    // html_lexer only, process token before emitting
    virtual void finalize() = 0;

protected:
    // set token type, only visible by subtype
    void set_type(token_type type) {_type = type;}

    // html_lexer only, set position [start, end) of original html
    // class html_text_token could reset positon on trim leading/trailing spaces
    void set_start_position(size_t pos) {_start = pos;}
    void set_end_position(size_t pos) {_end = pos;}

public:
    html_token() : _start(0), _end(0), _type(token_null) {}
    virtual ~html_token() {} // to invoke delete from base class

    // get token type
    token_type get_type() {return _type;}

    // get position [start, end) of original html
    size_t get_start_position() {return _start;}
    size_t get_end_position() {return _end;}

    // print tokenized information
    virtual void print() = 0;

    // print token in original html
    void print(std::string &html)
    {
        std::cerr << '[' << _start << ", " << _end << ") "
                  << html.substr(_start, _end - _start) << '\n';
    }
};

// abstract class for start and end tag tokens
class html_tag_token : public html_token
{
    friend class html_lexer;

private:
    // the name of start/end tag
    std::string _tag_name;

    // html_lexer only, append character to name
    void append_to_name(char c) {_tag_name.push_back(c);}

    // the lexer accept self-closing for both start and end tag.
    // end tag should override the function, emit error or omit it.
    virtual void set_self_closing() = 0;

    // html_lexer only, add new attribute
    virtual void new_attribute() = 0;

    // html_lexer only, append character to attribute name/value
    virtual void append_to_attribute_name(char c) = 0;
    virtual void append_to_attribute_value(char c) = 0;

protected:
    // set tag name
    void set_name(std::string &name) {_tag_name = name;}

public:
    // get tag name
    std::string get_name() {return _tag_name;}
};

// start tag token
class html_start_tag_token : public html_tag_token
{
    friend class html_lexer;

private:
    // attributes
    std::vector<std::pair<std::string, std::string>> _attributes;

    // html_lexer only, temp variables for attribute name/value
    std::string _attribute_name;
    std::string _attribute_value;

    // classes, class="..."
    std::set<std::string> _classes;

    // self-closing
    bool _is_self_closing;

    // html_lexer only, set self-closing
    void set_self_closing() {_is_self_closing = true;}

    // set classes
    void set_classes(std::string &classes)
    {
        split_classes_to_set(classes, _classes);
    }

    // html_lexer only, add new attribute and push temp attribute to attributes
    void new_attribute();

    // html_lexer only, append character to attribute name/value
    void append_to_attribute_name(char c) {_attribute_name.push_back(c);}
    void append_to_attribute_value(char c) {_attribute_value.push_back(c);}

    // html_lexer only, before emitting, push new attribute into set
    void finalize() {new_attribute();}

    // static function, split classes into set
    static void split_classes_to_set(
        std::string &classes, std::set<std::string> &classes_set);

public:
    html_start_tag_token() : _is_self_closing(false)
    {
        set_type(token_start_tag);
    }

    // get self-closing
    bool get_self_closing() {return _is_self_closing;}

    // check if tag has specific classes
    bool has_classes(std::string &classes);
    bool has_classes(std::set<std::string> &classes_set);

    // print tokenized information
    void print();
};

// end tag token
class html_end_tag_token : public html_tag_token
{
    friend class html_lexer;

private:
    // html_lexer only, end tag should not have attribute, omit if any.
    void new_attribute() {} // parse error
    void append_to_attribute_name(char c) {} // parse error
    void append_to_attribute_value(char c) {} // parse error

    // html_lexer only, end tag should not self-closing
    void set_self_closing() {} // parse error

    // html_lexer only, nothing to finalize for end tag
    void finalize() {}

public:
    html_end_tag_token() {set_type(token_end_tag);}

    // print tokenized information
    void print()
    {
        std::cout << "[End Tag        ] </" << get_name() << ">\n";
    }
};

// abstract class for text and comment tokens
class html_data_token : public html_token
{
    friend class html_lexer;

private:
    // html_lexer only, set content
    void set_content(std::string &content) {_data = content;}

    // html_lexer only, by default nothing to finalize for data token
    void finalize() {}

protected:
    // text or comment
    std::string _data;

public:
    // get content
    const std::string &get_readonly_content() {return _data;}

    // get content size
    size_t get_content_size() {return _data.size();}
};

// comment token
class html_comment_token : public html_data_token
{
public:
    html_comment_token() {set_type(token_comment);}

    // print tokenized information
    void print()
    {
        std::cout << "[Comment        ] <!--"
                  << get_readonly_content() << "-->\n";
    }
};

// bogus comment token
class html_bogus_comment_token : public html_data_token
{
public:
    html_bogus_comment_token() {set_type(token_bogus_comment);}

    // print tokenized information
    void print()
    {
        std::cout << "[Bogus Comment  ] " << get_readonly_content() << '\n';
    }
};

// text token
class html_text_token : public html_data_token
{
    friend class html_lexer;

private:
    // html_lexer only, append character to text
    void append_to_content(char c) {_data.push_back(c);}

    // html_lexer only, remove leading and trailing spaces
    void finalize();

public:
    html_text_token() {set_type(token_text);}

    // print tokenized information
    void print()
    {
        std::cout << "[Text           ] " << get_readonly_content() << '\n';
    }
};

// raw text token
class html_raw_text_token : public html_data_token
{
public:
    html_raw_text_token() {set_type(token_raw_text);}

    // print tokenized information
    void print()
    {
        std::cout << "[Raw Text       ] " << get_readonly_content() << '\n';
    }
};

class html_lexer
{
private:
    // state
    enum state_type
    {
        state_data,
        state_tag_open,
        state_end_tag_open,
        state_tag_name,
        state_self_closing_start_tag,
        state_before_attribute_name,
        state_attribute_name,
        state_after_attribute_name,
        state_before_attribute_value,
        state_attribute_value_unquoted,
        state_attribute_value_single_quoted,
        state_attribute_value_double_quoted,
        state_after_attribute_value_quoted,
        state_bogus_comment,
        state_markup_declaration_open
    };

    // a copy of html
    std::string _html;

    // the size of html
    size_t _size;

    // for state machine, tokenize(), process_...() functions
    // current processing position of html
    size_t _idx;
    // state machine state
    state_type  _state;
    // new token
    html_token *_token;

    // all tokens
    std::vector<html_token *>  _tokens;

    // finalize new token and add it to token vector
    void emit_token(size_t token_end_position);

    // release memory
    void clear_tokens()
    {
        for (auto token : _tokens)
        {
            delete token;
        }
        _tokens.clear();
    }

    // process raw text
    void process_raw_text(std::string &tag_name);

    // process markup declaration, <!-- -->, <[CDATA[...]]>, or <!doctype>
    void process_markup_declaration(size_t tag_start_position);

    // process bogus comment <!...> or <?...>
    void process_bogus_comment(size_t tag_start_position);

public:
    // constructor
    html_lexer() {};
    html_lexer(std::string &html) {tokenize(html);}

    // destructor
    ~html_lexer() {clear_tokens();}

    // npos for not found
    static const size_t npos = -1;

    // tokenizer, state machine
    bool tokenize(std::string &html);

    // return the number of tokens
    size_t size() {return _tokens.size();}

    // get nth token, return nullptr if out of range
    html_token *get_token(size_t pos);

    // find tag by name, return npos if not found
    size_t find_tag_by_name(std::string tag_name, bool start_tag, size_t pos);

    // find tag by name and classes, return npos if not found
    size_t find_tag_by_class_names(std::string tag_name,
                                   std::string classes, size_t pos);

    // find matching tag of nth tag
    // return pos, if nth tag is self-closing tag or no match tag
    // return position before pos, if nth tag is close tag
    // return position after pos, if nth tag is start tag
    size_t find_matching_tag(size_t pos);

    // print tokenized information
    void print()
    {
        for (auto token : _tokens)
        {
            token->print();
        }
        std::cout.flush();
    }

    // print original html of nth element
    void print(size_t pos)
    {
        if (pos < _tokens.size())
        {
            _tokens[pos]->print(_html);
        }
        std::cout.flush();
    }
};

#endif // __HTML_LEXER__
