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
    size_t                  line;  // for error message
    std::string::size_type  start; // [start, end)
    std::string::size_type  end;   // [start, end)

public:
    html_token() : line(0), start(0), end(0) {}
    virtual ~html_token() {};

    virtual token_type type() = 0;
    virtual void print() = 0;

    void set_start(std::string::size_type pos) {start = pos;}
    void set_end(std::string::size_type pos) {end = pos;}
};

class html_tag_token : public html_token
{
private:
    std::string                        tag_name;
    //std::map<std::string, std::string> attributes;
    std::vector<std::pair<std::string, std::string>> attributes;
    bool                               is_start_tag;
    bool                               is_self_closing;
    std::string                        attribute_name;
    std::string                        attribute_value;

public:
    html_tag_token(bool start_tag = true)
        : is_self_closing(false)
    {
        is_start_tag = start_tag;
    }

    token_type type() {return token_start_tag;}

    bool start_tag() {return is_start_tag;}

    void set_name(std::string &name) {tag_name = name;}
    std::string get_name() {return tag_name;}
    void append_to_name(char c) {tag_name.push_back(c);}

    void append_to_attribute_name(char c) {attribute_name.push_back(c);}
    void append_to_attribute_value(char c) {attribute_value.push_back(c);}
    void insert_attribute()
    {
        if (is_start_tag)
        {
            if (attribute_name.size() != 0)
            {
                attributes.push_back(std::make_pair(attribute_name, attribute_value));
                attribute_name.clear();
                attribute_value.clear();
            }
        }
        else
        {
            // error
                attribute_name.clear();
                attribute_value.clear();
        }
    }

    void set_self_closing()
    {
        if (is_start_tag)
        {
            is_self_closing = true;
        }
        else
        {
            // errormake clean
        }
    }

    void print()
    {
        if (is_start_tag)
        {
            std::cout << "[Start Tag      ] <" << tag_name << '\n';
            for (auto attribute : attributes)
            {
                std::cout << "[Attribute Name ] " << attribute.first << '\n';
                if (attribute.second.size() != 0)
                {
                    std::cout << "[Equal Sign     ] =\n";
                    std::cout << "[Attribute Value] " << attribute.second << '\n';
                }
            }
            if (is_self_closing)
            {
                std::cout << "[Self-closing   ] /\n";
            }
            std::cout << "[Tag End        ] >\n";
        }
        else
        {
            std::cout << "[End Tag        ] </" << tag_name << '\n';
            std::cout << "[Tag End        ] >\n";
        }
    }
};

class html_comment_token : public html_token
{
private:
    std::string data;

public:
    html_comment_token() {}

    token_type type() {return token_comment;}

    void set_comment(std::string &comment) {data = comment;}

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

    token_type type() {return token_text;}

    void append(char c) {data.push_back(c);}
    void set_text(std::string &text) {data = text;}

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
        state_after_attribute_value_quoted,
        state_raw_text
    };

    // errors
    enum error_type
    {
        error_invalid
    };

    std::string               _html;
    std::vector<html_token *> tokens;

public:
    html_lexer() {};
    html_lexer(std::string &html) {parse(html);}
    ~html_lexer()
    {
        for (auto token : tokens)
        {
            delete token;
        }
    }

    bool parse(std::string &html);

    void print()
    {
        for (auto token : tokens)
        {
            token->print();
        }
    }
};
