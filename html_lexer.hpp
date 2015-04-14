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

// abstract base class for html tokens
class html_token
{
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
    size_t     _start; // [start, end)
    size_t     _end;   // [start, end)
    token_type _type;

public:
    html_token() : _start(0), _end(0), _type(token_null) {}
    virtual ~html_token() {}

    // setter
    void set_type(token_type type) {_type = type;}
    void set_start_position(size_t pos) {_start = pos;}
    void set_end_position(size_t pos) {_end = pos;}

    // getter
    token_type get_type() {return _type;}
    size_t get_start_position() {return _start;}
    size_t get_end_position() {return _end;}

    // process token before emitting
    virtual void finalize() = 0;

    virtual void print() = 0;

    void print(std::string html)
    {
        std::cout << '[' << _start << ", " << _end << ") " << html.substr(_start, _end - _start) << '\n';
    }
};

// abstract class for start and end tag tokens
class html_tag_token : public html_token
{
private:
    std::string _tag_name;

public:
    // setter
    void set_name(std::string name) {_tag_name = name;}
    void append_to_name(char c) {_tag_name.push_back(c);}
    virtual void new_attribute() = 0;
    virtual void append_to_attribute_name(char c) = 0;
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
    html_start_tag_token() : _is_self_closing(false)
    {
        set_type(token_start_tag);
    }

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
    void append_to_attribute_name(char c) {_attribute_name.push_back(c);}
    void append_to_attribute_value(char c) {_attribute_value.push_back(c);}
    void set_self_closing() {_is_self_closing = true;}

    void finalize()
    {
        // push the last attribute
        new_attribute();
    }

    void print()
    {
        std::cout << "[Start Tag      ] <" << get_name();
        for (auto attribute : _attributes)
        {
            std::cout << ' ' << attribute.first;
            if (attribute.second.size() != 0)
            {
                std::cout << "='" << attribute.second << "'";
            }
        }
        if (_is_self_closing)
        {
            std::cout << "/";
        }
        std::cout << ">\n";
    }
};

class html_end_tag_token : public html_tag_token
{
public:
    html_end_tag_token()
    {
        set_type(token_end_tag);
    }

    // setter
    void new_attribute() {} // parse error
    void append_to_attribute_name(char c) {} // parse error
    void append_to_attribute_value(char c) {} // parse error
    void set_self_closing() {} // parse error

    void finalize() {}

    void print()
    {
        std::cout << "[End Tag        ] </" << get_name() << ">\n";
    }
};

// abstract class for text and comment tokens
class html_data_token : public html_token
{
private:
    std::string _data;

public:
    // setter
    void set_content(std::string &content) {_data = content;}

    // getter
    const std::string &get_readonly_content() {return _data;}
    std::string &get_writable_content() {return _data;}
    size_t get_content_size() {return _data.size();}
};

class html_comment_token : public html_data_token
{
public:
    html_comment_token() {set_type(token_comment);}

    void finalize() {}

    void print()
    {
        std::cout << "[Comment        ] <!--" << get_readonly_content() << "-->\n";
    }
};

class html_bogus_comment_token : public html_data_token
{
public:
    html_bogus_comment_token() {set_type(token_bogus_comment);}

    void finalize() {}

    void print()
    {
        std::cout << "[Bogus Comment  ] " << get_readonly_content() << '\n';
    }
};

class html_text_token : public html_data_token
{
public:
    html_text_token() {set_type(token_text);}

    void append_to_content(char c) {get_writable_content().push_back(c);}

    void finalize()
    {
        std::string &content = get_writable_content();

        // remove leading spaces
        auto pos = content.find_first_not_of("\t\r\n ");
        set_start_position(get_start_position() + pos);
        content.erase(0, pos);

        // remove trailing spaces
        pos = content.find_last_not_of("\t\r\n ");
        set_end_position(get_end_position() - (content.size() - pos - 1));
        content.erase(pos + 1, content.size());
    }

    void print()
    {
        std::cout << "[Text           ] " << get_readonly_content() << '\n';
    }
};

class html_raw_text_token : public html_data_token
{
public:
    html_raw_text_token() {set_type(token_raw_text);}

    void finalize() {}

    void print()
    {
        std::cout << "[Raw Text       ] " << get_readonly_content() << '\n';
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
        state_after_attribute_value_quoted,
        state_bogus_comment,
        state_markup_declaration_open
    };

    std::string                _html;
    size_t                     _size;
    size_t                     _idx;
    char                       _c;
    state_type                 _state;
    html_token                *_token;
    std::vector<html_token *>  _tokens;

    void clear_tokens()
    {
        for (auto token : _tokens)
        {
            delete token;
        }
        _tokens.clear();
    }

    void emit_token(size_t end_position);
    void process_raw_text(std::string tag_name);
    void process_markup_declaration(size_t start_position);
    void process_bogus_comment(size_t start_position);

public:
    html_lexer() {};
    html_lexer(std::string &html) {parse(html);}
    ~html_lexer()
    {
        clear_tokens();
    }

    bool parse(std::string &html);

    void print()
    {
        for (auto token : _tokens)
        {
            token->print();
            // token->print(_html);
        }
        std::cout.flush();
    }
};
