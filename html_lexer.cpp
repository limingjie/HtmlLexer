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

void html_lexer::emit_token(size_t end_position)
{
    if (_token != nullptr)
    {
        _token->set_end_position(end_position);
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

void html_lexer::process_raw_text(std::string tag_name)
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
                if (c == '>' || c == '\t' || c == '\r' || c == '\n' || c == ' ')
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

void html_lexer::process_markup_declaration(size_t start_position)
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
        _token->set_start_position(start_position);

        auto pos = _html.find("]]>", _idx + 7);
        std::string cdata;
        if (pos == std::string::npos)
        {
            cdata = _html.substr(start_position);
            _idx = _size - 1; // point to the last char of html
        }
        else
        {
            cdata = _html.substr(start_position, pos + 3 - start_position);
            _idx = pos + 2; // point to '>'
        }
        ((html_raw_text_token *)_token)->set_content(cdata);

        _state = state_data;
        emit_token(_idx + 1);
    }
    else if (iequals(_html.substr(_idx, 7), "DOCTYPE"))
    {
        _token = new html_start_tag_token();
        _token->set_start_position(start_position);
        ((html_start_tag_token *)_token)->set_name("!doctype");
        _idx += 6; // point to 'E'
        _state = state_tag_name;
    }
    else
    {
        process_bogus_comment(start_position);
    }
}

void html_lexer::process_bogus_comment(size_t start_position)
{
    // emit raw text token
    html_bogus_comment_token *token = new html_bogus_comment_token();
    token->set_start_position(start_position);

    auto pos = _html.find(">", _idx);
    if (pos == std::string::npos)
    {
        pos = _size - 1;
    }

    std::string bogus_comment = _html.substr(start_position, pos - start_position + 1);
    token->set_end_position(pos + 1);
    token->set_content(bogus_comment);
    token->finalize();
    _tokens.push_back(token);

    _idx = pos; // point to '>' or the last char of html
    _state = state_data;
}

bool html_lexer::parse(std::string &html)
{
    _html  = html; // Copy
    _size  = _html.size();
    _idx   = 0;
    _state = state_data;
    _token = nullptr;

    clear_tokens();

    size_t pos_tag_open = 0;

    while (_idx < _size)
    {
        _c = _html[_idx];

        switch (_state)
        {
        // http://www.w3.org/TR/html5/syntax.html#data-_state
        case state_data:
			// std::cerr << "state_data" << std::endl;
            if (_c == '<')
            {
                // remember tag open position
                pos_tag_open = _idx;
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

                ((html_text_token *)_token)->append_to_content(_c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#tag-open-_state
        case state_tag_open:
			// std::cerr << "state_tag_open" << std::endl;
            if (_c == '!')
            {
                _state = state_markup_declaration_open;
            }
            else if (_c == '/')
            {
                _state = state_end_tag_open;
            }
            else if (isupper(_c))
            {
                _token = new html_start_tag_token();
                _token->set_start_position(pos_tag_open);
                ((html_tag_token *)_token)->append_to_name(tolower(_c));
                _state = state_tag_name;
            }
            else if (islower(_c))
            {
                _token = new html_start_tag_token();
                _token->set_start_position(pos_tag_open);
                ((html_tag_token *)_token)->append_to_name(_c);
                _state = state_tag_name;
            }
            else if (_c == '?')
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
            if (isupper(_c))
            {
                _token = new html_end_tag_token();
                _token->set_start_position(pos_tag_open);
                ((html_tag_token *)_token)->append_to_name(tolower(_c));
                _state = state_tag_name;
            }
            else if (islower(_c))
            {
                _token = new html_end_tag_token();
                _token->set_start_position(pos_tag_open);
                ((html_tag_token *)_token)->append_to_name(_c);
                _state = state_tag_name;
            }
            else if (_c == '>')
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
            if (_c == '\t' || _c == '\r' || _c == '\n' || _c == ' ')
            {
                _state = state_before_attribute_name;
            }
            else if (_c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (_c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else if (isupper(_c))
            {
                ((html_tag_token *)_token)->append_to_name(tolower(_c));
            }
            else
            {
                ((html_tag_token *)_token)->append_to_name(_c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#before-attribute-name-_state
        case state_before_attribute_name:
			// std::cerr << "state_before_attribute_name" << std::endl;
            if (_c == '\t' || _c == '\r' || _c == '\n' || _c == ' ')
            {
                // ignore
            }
            else if (_c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (_c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else if (isupper(_c))
            {
                ((html_tag_token *)_token)->new_attribute();
                ((html_tag_token *)_token)->append_to_attribute_name(tolower(_c));
                _state = state_attribute_name;
            }
            else
            {
                // error
                if (_c == '"' || _c == '\'' || _c == '<' || _c == '=')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)_token)->new_attribute();
                ((html_tag_token *)_token)->append_to_attribute_name(_c);
                _state = state_attribute_name;
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#attribute-name-_state
        case state_attribute_name:
			// std::cerr << "state_attribute_name" << std::endl;
            if (_c == '\t' || _c == '\r' || _c == '\n' || _c == ' ')
            {
                _state = state_after_attribute_name;
            }
            else if (_c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (_c == '=')
            {
                _state = state_before_attribute_value;
            }
            else if (_c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else if (isupper(_c))
            {
                ((html_tag_token *)_token)->append_to_attribute_name(tolower(_c));
            }
            else
            {
                // error
                if (_c == '"' || _c == '\'' || _c == '<')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)_token)->append_to_attribute_name(_c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#after-attribute-name-_state
        case state_after_attribute_name:
			// std::cerr << "state_after_attribute_name" << std::endl;
            if (_c == '\t' || _c == '\r' || _c == '\n' || _c == ' ')
            {
                // ignore
            }
            else if (_c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (_c == '=')
            {
                _state = state_before_attribute_value;
            }
            else if (_c =='>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else if (isupper(_c))
            {
                ((html_tag_token *)_token)->new_attribute();
                ((html_tag_token *)_token)->append_to_attribute_name(tolower(_c));
                _state = state_attribute_name;
            }
            else
            {
                // error
                if (_c == '"' || _c == '\'' || _c == '<')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)_token)->new_attribute();
                ((html_tag_token *)_token)->append_to_attribute_name(_c);
                _state = state_attribute_name;
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#before-attribute-value-_state
        case state_before_attribute_value:
			// std::cerr << "state_before_attribute_value" << std::endl;
            if (_c == '\t' || _c == '\r' || _c == '\n' || _c == ' ')
            {
                // ignore
            }
            else if (_c == '"')
            {
                _state = state_attribute_value_double_quoted;
            }
            else if (_c == '\'')
            {
                _state = state_attribute_value_single_quoted;
            }
            else if (_c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else
            {
                // error
                if (_c == '<' || _c == '=' || _c == '`')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)_token)->append_to_attribute_value(_c);
                _state = state_attribute_value_unquoted;
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#attribute-value-(double-quoted)-_state
        case state_attribute_value_double_quoted:
			// std::cerr << "state_attribute_value_double_quoted" << std::endl;
            if (_c == '"')
            {
                _state = state_after_attribute_value_quoted;
            }
            else
            {
                ((html_tag_token *)_token)->append_to_attribute_value(_c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#attribute-value-(single-quoted)-_state
        case state_attribute_value_single_quoted:
			// std::cerr << "state_attribute_value_single_quoted" << std::endl;
            if (_c == '\'')
            {
                _state = state_after_attribute_value_quoted;
            }
            else
            {
                ((html_tag_token *)_token)->append_to_attribute_value(_c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#attribute-value-(unquoted)-_state
        case state_attribute_value_unquoted:
			// std::cerr << "state_attribute_value_unquoted" << std::endl;
            if (_c == '\t' || _c == '\r' || _c == '\n' || _c == ' ')
            {
                _state = state_before_attribute_name;
            }
            else if (_c == '>')
            {
                _state = state_data;
                emit_token(_idx + 1);
            }
            else
            {
                // error
                if (_c == '"' || _c == '\'' || _c == '<' || _c == '=' || _c == '`')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)_token)->append_to_attribute_value(_c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#after-attribute-value-(quoted)-_state
        case state_after_attribute_value_quoted:
			// std::cerr << "state_after_attribute_value_quoted" << std::endl;
            if (_c == '\t' || _c == '\r' || _c == '\n' || _c == ' ')
            {
                _state = state_before_attribute_name;
            }
            else if (_c == '/')
            {
                _state = state_self_closing_start_tag;
            }
            else if (_c == '>')
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
            if (_c == '>')
            {
                _state = state_data;
                ((html_tag_token *)_token)->set_self_closing();
                emit_token(_idx + 1);
            }
            else
            {
                // error
                _state = state_before_attribute_name;
                continue; // reconsume the character
            }
            break;
        case state_bogus_comment:
            process_bogus_comment(pos_tag_open);
            break;
        case state_markup_declaration_open:
            process_markup_declaration(pos_tag_open);
            break;
        default:
            // std::cerr << "what is this?" << std::endl;
            break;
        }

        ++_idx; // consume next char
    }

    return true;
}
