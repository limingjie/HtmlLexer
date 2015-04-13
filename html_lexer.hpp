//
// HTML Lexer
// A HTML Lexer/Tokenizer which follows HTML5 specification
//
// Author - Mingjie Li
// Email  - limingjie@outlook.com
//          limingjie0719@gmail.com
// Date   - Apr 12, 2015
// Github - https://github.com/limingjie/HtmlLexer
//

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cctype> // tolower(), isupper(), islower()

enum token_type
{
    token_start_tag,
    token_end_tag,
    token_comment,
    token_text
};

class html_token
{
private:
    std::string::size_type  _start; // [start, end)
    std::string::size_type  _end;   // [start, end)

public:
    html_token() : _start(0), _end(0) {}
    virtual ~html_token() {};

    // setter
    void set_start_position(std::string::size_type pos) {_start = pos;}
    void set_end_position  (std::string::size_type pos) {_end   = pos;}

    // getter
    virtual token_type get_type() = 0;

    virtual void finalize() = 0;
    virtual void print()    = 0;
};

class html_tag_token : public html_token
{
private:
    std::string _tag_name;

public:
    html_tag_token() {}

    // setter
    void append_to_name(char c) {_tag_name.push_back(c);}
    virtual void new_attribute() = 0;
    virtual void append_to_attribute_name (char c) = 0;
    virtual void append_to_attribute_value(char c) = 0;
    virtual void set_self_closing() = 0;

    // getter
    std::string get_name() {return _tag_name;}
};

class html_start_tag_token : public html_tag_token
{
private:
    std::vector<std::pair<std::string, std::string>> _attributes;
    std::string                                      _attribute_name;
    std::string                                      _attribute_value;
    bool                                             _is_self_closing;

public:
    html_start_tag_token() : _is_self_closing(false) {}

    // setter
    void new_attribute()
    {
        if (_attribute_name.size() != 0)
        {
            _attributes.push_back(std::make_pair(_attribute_name, _attribute_value));
        }

        _attribute_name.clear();
        _attribute_value.clear();
    }
    void append_to_attribute_name (char c) {_attribute_name.push_back(c);}
    void append_to_attribute_value(char c) {_attribute_value.push_back(c);}
    void set_self_closing() {_is_self_closing = true;}

    // getter
    token_type  get_type() {return token_start_tag;}

    void finalize ()
    {
        // push attribute
        new_attribute();
    }

    void print()
    {
        std::cout << "[Start Tag      ] <" << get_name() << '\n';
        for (auto attribute : _attributes)
        {
            std::cout << "[Attribute Name ] " << attribute.first << '\n';
            if (attribute.second.size() != 0)
            {
                std::cout << "[Equal Sign     ] =\n";
                std::cout << "[Attribute Value] " << attribute.second << '\n';
            }
        }
        if (_is_self_closing)
        {
            std::cout << "[Self-closing   ] /\n";
        }
        std::cout << "[Tag End        ] >\n";
    }
};

class html_end_tag_token : public html_tag_token
{
public:
    // setter
    void new_attribute() {} // parse error
    void append_to_attribute_name (char c) {} // parse error
    void append_to_attribute_value(char c) {} // parse error
    void set_self_closing() {} // parse error

    // getter
    token_type  get_type() {return token_end_tag;}

    void finalize () {}

    void print()
    {
        std::cout << "[End Tag        ] </" << get_name() << '\n';
        std::cout << "[Tag End        ] >\n";
    }
};

class html_comment_token : public html_token
{
private:
    std::string data;

public:
    html_comment_token() {}

    // setter
    void set_comment(std::string &comment) {data = comment;}

    // getter
    token_type get_type() {return token_comment;}

    void finalize() {}

    void print()
    {
        std::cout << "[Comment        ] <!--" << data << "-->\n";
    }
};

class html_text_token : public html_token
{
private:
    std::string data;

public:
    html_text_token() {}

    // setter
    void append(char c) {data.push_back(c);}
    void set_text(std::string &text) {data = text;}

    // getter
    token_type get_type() {return token_text;}

    void finalize() {}

    void print()
    {
        // std::cout << "[Text           ] " << data << '\n';
    }
};

class html_lexer
{
private:
    // states
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
        state_after_attribute_value_quoted
    };

    // errors
    enum error_type
    {
        error_invalid
    };

    std::string                _html;
    size_t                     _size;
    size_t                     _idx;
    char                       _c;
    state_type                 _state;
    html_token                *_token;
    std::vector<html_token *>  _tokens;

public:
    html_lexer() {};
    html_lexer(std::string &html) {parse(html);}
    ~html_lexer()
    {
        for (auto token : _tokens)
        {
            delete token;
        }
    }

    bool parse(std::string &html);
    void emit_token(std::string::size_type end_position);
    void process_raw_text(std::string tag_name);

    void print()
    {
        for (auto token : _tokens)
        {
            token->print();
            std::cout.flush();
        }
    }
};
