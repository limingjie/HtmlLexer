#include <string>
#include <vector>
#include <map>

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
    size_t                  line;
    std::string::size_type  start; // [start, end)
    std::string::size_type  end;   // [start, end)

public:
    type() : line(0), start(0), end(0) {}
    virtual token_type type() = 0;
};

class html_token_start_tag : public html_token
{
private:
    std::string                        tag_name;
    bool                               self_closing;
    std::map<std::string, std::string> attributes;

public:
    html_token_start_tag();

    token_type type() {return token_start_tag;}
};

class html_token_end_tag : public html_token
{
private:
    std::string                        tag_name;
    bool                               self_closing;
    std::map<std::string, std::string> attributes;

public:
    html_token_end_tag();

    token_type type() {return token_end_tag;}
};

class html_token_comment : public html_token
{
private:
    std::string data;

public:
    html_token_comment();

    token_type type() {return token_comment;}
};

class html_token_text : public html_token
{
private:
    std::string data;

public:
    html_token_text();

    token_type type() {return token_text;}
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

    std::vector<html_token> tokens;

};
