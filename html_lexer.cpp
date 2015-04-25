#include "html_lexer.hpp"

// compare string case insensitive
static bool iequals(const std::string &str1, const std::string &str2)
{
    if (str1.size() != str2.size())
    {
        return false;
    }

    auto c1  = str1.cbegin();
    auto c2  = str2.cbegin();
    auto end = str1.cend();

    while (c1 != end)
    {
        if (tolower(*c1) != tolower(*c2))
        {
            return false;
        }

        ++c1;
        ++c2;
    }

    return true;
}

//
// class html_start_tag_token methods
//

// lexer only, add new attribute and push temp attribute to attribute vector
void html_start_tag_token::new_attribute()
{
    if (_attribute_name.size() != 0)
    {
        _attributes.push_back(
            std::make_pair(_attribute_name, _attribute_value));

        if (_attribute_name == "class")
        {
            set_classes(_attribute_value);
        }

        _attribute_name.clear();
    }

    _attribute_value.clear();
}

// split classes into set
void html_start_tag_token::split_classes_to_set(
    std::string &classes, std::set<std::string> &classes_set)
{
    size_t first;
    size_t last = 0;

    while (last != std::string::npos)
    {
        first = classes.find_first_not_of(" \n\r\t", last);
        if (first == std::string::npos) break;

        last = classes.find_first_of(" \n\r\t", first + 1);
        if (last == std::string::npos)
        {
            classes_set.insert(classes.substr(first));
        }
        else
        {
            classes_set.insert(classes.substr(first, last - first));
        }
    }
}

// check if tag has specific classes
bool html_start_tag_token::has_classes(std::string &classes)
{
    std::set<std::string> classes_set;
    split_classes_to_set(classes, classes_set);

    return has_classes(classes_set);
}

// check if tag has specific classes
bool html_start_tag_token::has_classes(std::set<std::string> &classes_set)
{
    for (auto it = classes_set.cbegin(); it != classes_set.cend(); ++it)
    {
        // TODO: case sensitive?
        if (_classes.find(*it) == _classes.end())
        {
            return false;
        }
    }

    return true;
}

// print tokenized information
void html_start_tag_token::print()
{
    std::cout << "[Start Tag      ] <" << get_name();
    for (auto attribute : _attributes)
    {
        std::cout << ' ' << attribute.first;
        if (attribute.second.size() != 0)
        {
            std::cout << "=\"" << attribute.second << "\"";
        }
    }
    if (_is_self_closing)
    {
        std::cout << "/";
    }
    std::cout << ">\n";
}

//
// class html_text_token methods
//

// lexer only, remove leading and trailing spaces
void html_text_token::finalize()
{
    // remove leading spaces
    auto pos = _data.find_first_not_of(" \n\r\t");
    set_start_position(get_start_position() + pos);
    _data.erase(0, pos);

    // remove trailing spaces
    pos = _data.find_last_not_of(" \n\r\t");
    set_end_position(get_end_position() - (_data.size() - pos - 1));
    _data.erase(pos + 1, _data.size());
}

//
// class html_lexer methods
//

// add new token to token vector
void html_lexer::emit_token(size_t token_end_position)
{
    if (_token != nullptr)
    {
        _token->set_end_position(token_end_position);
        _token->finalize();

        html_token::token_type type = _token->get_type();
        if (type == html_token::token_start_tag)
        {
            _tokens.push_back(_token);
            std::string tag_name = ((html_tag_token *)_token)->get_name();
            if (tag_name == "textarea" || tag_name == "style" ||
                tag_name == "script"   || tag_name == "title" )
            {
                process_raw_text(tag_name);
            }
        }
        else if (type == html_token::token_text)
        {
            if (((html_text_token *)_token)->get_content_size() == 0)
            {
                delete _token;
            }
            else
            {
                _tokens.push_back(_token);
            }
        }
        else
        {
            _tokens.push_back(_token);
        }

        // do not delete token here, as token is pushed into _tokens[]
        _token = nullptr;
    }
}

// process raw text
void html_lexer::process_raw_text(std::string &tag_name)
{
    char c;
    std::string name;

    auto size = tag_name.size();
    if (size == 0) return;

    // move after '>'
    auto pos = _idx + 1;

    while (true)
    {
        // find position for possible end tag
        pos = _html.find("</", pos);
        if (pos != std::string::npos)
        {
            // match name
            name = _html.substr(pos + 2, size);
            if (iequals(tag_name, name))
            {
                // It is the end tag if the name is followed by '>' or space
                c = _html[pos + 2 + size];
                if (c == '>' || c == ' ' || c == '\n' || c == '\r' || c == '\t')
                {
                    break;
                }
            }

            pos += 2;
            continue;
        }
        else
        {
            // Not found, treat all other chars as raw text
            pos = size;
        }

        break;
    }

    if (pos != _idx + 1)
    {
        ++_idx;

        // emit raw text token
        html_raw_text_token *token = new html_raw_text_token();
        std::string raw_text = _html.substr(_idx, pos - _idx);

        token->set_start_position(_idx);
        token->set_end_position(pos);
        token->set_content(raw_text);
        token->finalize();
        _tokens.push_back(token);

        _idx = pos - 1; // point to the char before '<' or the last char of html
    }
}

// process markup declaration, <!-- -->, <[CDATA[...]]>, or <!doctype>
void html_lexer::process_markup_declaration(size_t tag_start_position)
{
    if (_html[_idx] == '-' && _html[_idx + 1] == '-')
    {
        _token = new html_comment_token();
        _token->set_start_position(_idx - 2);

        auto pos = _html.find("-->", _idx + 2);
        std::string comment;
        if (pos == std::string::npos)
        {
            comment = _html.substr(_idx + 2);
            _idx = _size - 1; // point to the last char of html
        }
        else
        {
            comment = _html.substr(_idx + 2, pos - _idx - 2);
            _idx = pos + 2; // point to '>'
        }
        ((html_comment_token *)_token)->set_content(comment);

        _state = state_data;
        emit_token(_idx + 1);
    }
    else if (_html.substr(_idx, 7) == "[CDATA[")
    {
        _token = new html_raw_text_token();
        _token->set_start_position(tag_start_position);

        auto pos = _html.find("]]>", _idx + 7);
        std::string cdata;
        if (pos == std::string::npos)
        {
            cdata = _html.substr(tag_start_position);
            _idx = _size - 1; // point to the last char of html
        }
        else
        {
            cdata = _html.substr(tag_start_position,
                                 pos + 3 - tag_start_position);
            _idx = pos + 2; // point to '>'
        }
        ((html_raw_text_token *)_token)->set_content(cdata);

        _state = state_data;
        emit_token(_idx + 1);
    }
    else if (iequals(_html.substr(_idx, 7), "DOCTYPE"))
    {
        std::string tag_name("!doctype");
        _token = new html_start_tag_token();
        _token->set_start_position(tag_start_position);
        ((html_start_tag_token *)_token)->set_name(tag_name);
        _idx += 6; // point to 'E'
        _state = state_tag_name;
    }
    else
    {
        process_bogus_comment(tag_start_position);
    }
}

// process bogus comment
void html_lexer::process_bogus_comment(size_t tag_start_position)
{
    // emit raw text token
    html_bogus_comment_token *token = new html_bogus_comment_token();
    token->set_start_position(tag_start_position);

    auto pos = _html.find(">", _idx);
    if (pos == std::string::npos)
    {
        pos = _size - 1;
    }

    std::string bogus_comment =
        _html.substr(tag_start_position, pos - tag_start_position + 1);
    token->set_end_position(pos + 1);
    token->set_content(bogus_comment);
    token->finalize();
    _tokens.push_back(token);

    _idx = pos; // point to '>' or the last char of html
    _state = state_data;
}

// tokenizer, state machine
bool html_lexer::tokenize(std::string &html)
{
    // reset state machine
    _html  = html; // copy
    _size  = _html.size();
    _idx   = 0;
    _state = state_data;
    _token = nullptr;
    clear_tokens();

    size_t tag_start_position = 0;
    char c;

    while (_idx < _size)
    {
        c = _html[_idx];

        switch (_state)
        {
        // http://www.w3.org/TR/html5/syntax.html#data-_state
        case state_data:
            // std::cerr << "state_data" << std::endl;
            if (c == '<')
            {
                // remember tag open position
                tag_start_position = _idx;
                _state = state_tag_open;
                emit_token(_idx);
            }
            else
            {
                if (_token == nullptr)
                {
                    _token = new html_text_token();
                    _token->set_start_position(_idx);
                }

                ((html_text_token *)_token)->append_to_content(c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#tag-open-_state
        case state_tag_open:
            // std::cerr << "state_tag_open" << std::endl;
            if (c == '!')
            {
                _state = state_markup_declaration_open;
            }
            else if (c == '/')
            {
                _state = state_end_tag_open;
            }
            else if (isupper(c))
            {
                _token = new html_start_tag_token();
                _token->set_start_position(tag_start_position);
                ((html_tag_token *)_token)->append_to_name(tolower(c));
                _state = state_tag_name;
            }
            else if (islower(c))
            {
                _token = new html_start_tag_token();
                _token->set_start_position(tag_start_position);
                ((html_tag_token *)_token)->append_to_name(c);
                _state = state_tag_name;
            }
            else if (c == '?')
            {
                _state = state_bogus_comment;
            }
            else
            {
                // parse error
                _state = state_data;
                continue; // reconsume current char
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#end-tag-open-_state
        case state_end_tag_open:
            // std::cerr << "state_end_tag_open" << std::endl;
            if (isupper(c))
            {
                _token = new html_end_tag_token();
                _token->set_start_position(tag_start_position);
                ((html_tag_token *)_token)->append_to_name(tolower(c));
                _state = state_tag_name;
            }
            else if (islower(c))
            {
                _token = new html_end_tag_token();
                _token->set_start_position(tag_start_position);
                ((html_tag_token *)_token)->append_to_name(c);
                _state = state_tag_name;
            }
            else if (c == '>')
            {
                _state = state_data;
            }
            else
            {
                // parse error
                _state = state_bogus_comment;
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#tag-name-_state
        case state_tag_name:
            // std::cerr << "state_tag_name" << std::endl;
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            {
                _state = state_before_attribute_name;
            }
            else if (c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else if (isupper(c))
            {
                ((html_tag_token *)_token)->append_to_name(tolower(c));
            }
            else
            {
                ((html_tag_token *)_token)->append_to_name(c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#before-attribute-name-_state
        case state_before_attribute_name:
            // std::cerr << "state_before_attribute_name" << std::endl;
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            {
                // ignore
            }
            else if (c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else if (isupper(c))
            {
                ((html_tag_token *)_token)->new_attribute();
                ((html_tag_token *)_token)->
                    append_to_attribute_name(tolower(c));
                _state = state_attribute_name;
            }
            else
            {
                // parse error
                if (c == '"' || c == '\'' || c == '<' || c == '=')
                {
                    // treat it as attribute name
                }

                ((html_tag_token *)_token)->new_attribute();
                ((html_tag_token *)_token)->append_to_attribute_name(c);
                _state = state_attribute_name;
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#attribute-name-_state
        case state_attribute_name:
            // std::cerr << "state_attribute_name" << std::endl;
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            {
                _state = state_after_attribute_name;
            }
            else if (c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (c == '=')
            {
                _state = state_before_attribute_value;
            }
            else if (c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else if (isupper(c))
            {
                ((html_tag_token *)_token)->
                    append_to_attribute_name(tolower(c));
            }
            else
            {
                // parse error
                if (c == '"' || c == '\'' || c == '<')
                {
                    // treat it as attribute name
                }

                ((html_tag_token *)_token)->append_to_attribute_name(c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#after-attribute-name-_state
        case state_after_attribute_name:
            // std::cerr << "state_after_attribute_name" << std::endl;
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            {
                // ignore
            }
            else if (c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (c == '=')
            {
                _state = state_before_attribute_value;
            }
            else if (c =='>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else if (isupper(c))
            {
                ((html_tag_token *)_token)->new_attribute();
                ((html_tag_token *)_token)->
                    append_to_attribute_name(tolower(c));
                _state = state_attribute_name;
            }
            else
            {
                // parse error
                if (c == '"' || c == '\'' || c == '<')
                {
                    // treat it as attribute name
                }

                ((html_tag_token *)_token)->new_attribute();
                ((html_tag_token *)_token)->append_to_attribute_name(c);
                _state = state_attribute_name;
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#before-attribute-value-_state
        case state_before_attribute_value:
            // std::cerr << "state_before_attribute_value" << std::endl;
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            {
                // ignore
            }
            else if (c == '"')
            {
                _state = state_attribute_value_double_quoted;
            }
            else if (c == '\'')
            {
                _state = state_attribute_value_single_quoted;
            }
            else if (c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else
            {
                // parse error
                if (c == '<' || c == '=' || c == '`')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)_token)->append_to_attribute_value(c);
                _state = state_attribute_value_unquoted;
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#attribute-value-(double-quoted)-_state
        case state_attribute_value_double_quoted:
            // std::cerr << "state_attribute_value_double_quoted" << std::endl;
            if (c == '"')
            {
                _state = state_after_attribute_value_quoted;
            }
            else
            {
                ((html_tag_token *)_token)->append_to_attribute_value(c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#attribute-value-(single-quoted)-_state
        case state_attribute_value_single_quoted:
            // std::cerr << "state_attribute_value_single_quoted" << std::endl;
            if (c == '\'')
            {
                _state = state_after_attribute_value_quoted;
            }
            else
            {
                ((html_tag_token *)_token)->append_to_attribute_value(c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#attribute-value-(unquoted)-_state
        case state_attribute_value_unquoted:
            // std::cerr << "state_attribute_value_unquoted" << std::endl;
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            {
                _state = state_before_attribute_name;
            }
            else if (c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else
            {
                // parse error
                if (c == '"' || c == '\'' || c == '<' || c == '=' || c == '`')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)_token)->append_to_attribute_value(c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#after-attribute-value-(quoted)-_state
        case state_after_attribute_value_quoted:
            // std::cerr << "state_after_attribute_value_quoted" << std::endl;
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            {
                _state = state_before_attribute_name;
            }
            else if (c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else
            {
                _state = state_before_attribute_name;
                continue; // reconsume the character
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#self-closing-start-tag-_state
        case state_self_closing_start_tag:
            // std::cerr << "state_self_closing_start_tag" << std::endl;
            if (c == '>')
            {
                _state = state_data;
                ((html_tag_token *)_token)->set_self_closing();
                emit_token(_idx + 1);
            }
            else
            {
                // parse error
                _state = state_before_attribute_name;
                continue; // reconsume the character
            }
            break;
        case state_bogus_comment:
            process_bogus_comment(tag_start_position);
            break;
        case state_markup_declaration_open:
            process_markup_declaration(tag_start_position);
            break;
        default:
            // std::cerr << "what is this?" << std::endl;
            break;
        }

        ++_idx; // consume next char
    }

    return true;
}

// get nth token, return nullptr if out of range
html_token *html_lexer::get_token(size_t pos)
{
    size_t size = _tokens.size();
    if (pos >= size) return nullptr;

    return _tokens[pos];
}


// find tag by name, return npos if not found
size_t html_lexer::find_tag_by_name(
    std::string tag_name, bool start_tag, size_t pos)
{
    size_t size = _tokens.size();
    if (pos >= size) return npos;

    html_token *token;
    html_token::token_type type;
    for (size_t idx = pos; idx < size; ++idx)
    {
        token = _tokens[idx];
        type = token->get_type();
        if ( ( start_tag && type == html_token::token_start_tag) ||
             (!start_tag && type == html_token::token_end_tag  ) )
        {
            if (iequals(((html_tag_token *)token)->get_name(), tag_name))
            {
                return idx;
            }
        }
    }

    return npos;
}

// find tag by name and classes, return npos if not found
size_t html_lexer::find_tag_by_class_names(
    std::string tag_name, std::string classes, size_t pos)
{
    size_t size = _tokens.size();
    if (pos >= size) return npos;

    html_token *token;
    html_token::token_type type;
    std::set<std::string> classes_set;
    html_start_tag_token::split_classes_to_set(classes, classes_set);

    for (size_t idx = pos; idx < size; ++idx)
    {
        token = _tokens[idx];
        type = token->get_type();
        if (type == html_token::token_start_tag)
        {
            if (iequals(((html_start_tag_token *)token)->get_name(), tag_name)
                && ((html_start_tag_token *)token)->has_classes(classes_set))
            {
                return idx;
            }
        }
    }

    return npos;
}

// find matching tag of nth tag
// return pos, if nth tag is self-closing tag or no match tag
// return position before pos, if nth tag is close tag
// return position after pos, if nth tag is start tag
size_t html_lexer::find_matching_tag(size_t pos)
{
    size_t size = _tokens.size();
    if (pos >= size) return npos;

    html_token *token = _tokens[pos];
    html_token::token_type type = token->get_type();
    std::string tag_name;
    html_token *token2;
    html_token::token_type type2;
    size_t depth = 0;

    if (type == html_token::token_start_tag)
    {
        if (((html_start_tag_token *)token)->get_self_closing())
        {
            return pos;
        }

        tag_name = ((html_tag_token *)token)->get_name();

        for (size_t i = pos + 1; i < size; ++i)
        {
            token2 = _tokens[i];
            type2 = token2->get_type();
            if (type2 == html_token::token_start_tag)
            {
                if (((html_tag_token *)token2)->get_name() == tag_name)
                {
                    ++depth;
                }
            }
            else if (type2 == html_token::token_end_tag)
            {
                if (((html_tag_token *)token2)->get_name() == tag_name)
                {
                    if (depth == 0)
                    {
                        return i;
                    }

                    --depth;
                }
            }
        }
    }
    else if (type == html_token::token_end_tag)
    {
        tag_name = ((html_tag_token *)token)->get_name();

        if (pos == 0) return 0;

        for (size_t i = pos - 1; i != npos; --i)
        {
            token2 = _tokens[i];
            type2 = token2->get_type();
            if (type2 == html_token::token_start_tag)
            {
                if (((html_tag_token *)token2)->get_name() == tag_name)
                {
                    if (depth == 0)
                    {
                        return i;
                    }

                    --depth;
                }
            }
            else if (type2 == html_token::token_end_tag)
            {
                if (((html_tag_token *)token2)->get_name() == tag_name)
                {
                    ++depth;
                }
            }
        }
    }

    return pos;
}
