#include "html_lexer.hpp"

void html_lexer::emit_token(std::string::size_type end_position)
{
    if (_token != nullptr)
    {
        _token->set_end_position(end_position);
        _token->finalize();
        _tokens.push_back(_token);

        if (_token->get_type() == token_start_tag)
        {
            std::string tag_name = ((html_tag_token *)_token)->get_name();

            if (tag_name == "textarea" || tag_name == "style" ||
                tag_name == "script"   || tag_name == "title" )
            {
                process_raw_text(tag_name);
            }
        }

        _token = nullptr;
    }
}

void html_lexer::process_raw_text(std::string tag_name)
{
    size_t first = _idx;
    while (true)
    {
        first = _html.find("</" + tag_name, first);
        // std::cerr << tag_name << std::endl;
        if (first != std::string::npos)
        {
            _idx = first - 1;
        }
        else
        {
            _idx = _size - 1;
        }

        break;
    }
}

bool html_lexer::parse(std::string &html)
{
    _html  = html; // Copy
    _size  = _html.size();
    _idx   = 0;
    _state = state_data;
    _token = nullptr;

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

                ((html_text_token *)_token)->append(_c);
            }
            break;

        // http://www.w3.org/TR/html5/syntax.html#tag-open-_state
        case state_tag_open:
			// std::cerr << "state_tag_open" << std::endl;
            if (_c == '!')
            {
                if (html[_idx + 1] == '-' && html[_idx + 2] == '-')
                {
                    _token = new html_comment_token();
                    _token->set_start_position(_idx - 1);

                    auto pos = _html.find("-->", _idx + 3);
                    std::string comment;
                    if (pos == std::string::npos)
                    {
                        comment = _html.substr(_idx + 3);
                        _idx = _size;
                    }
                    else
                    {
                        comment = _html.substr(_idx + 3, pos - _idx - 3);
                        _idx = pos + 3;
                    }
                    ((html_comment_token *)_token)->set_comment(comment);

                    emit_token(_idx);
                    _state = state_data;

                    continue;
                }
            }
            else if (_c == '/')
            {
                _state = state_end_tag_open;
            }
            else if (isupper(_c))
            {
                _token = new html_start_tag_token();
                _token->set_start_position(_idx);
                ((html_tag_token *)_token)->append_to_name(tolower(_c));
                _state = state_tag_name;
            }
            else if (islower(_c))
            {
                _token = new html_start_tag_token();
                _token->set_start_position(_idx);
                ((html_tag_token *)_token)->append_to_name(_c);
                _state = state_tag_name;
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
                _token->set_start_position(_idx);
                ((html_tag_token *)_token)->append_to_name(tolower(_c));
                _state = state_tag_name;
            }
            else if (islower(_c))
            {
                _token = new html_end_tag_token();
                _token->set_start_position(_idx);
                ((html_tag_token *)_token)->append_to_name(_c);
                _state = state_tag_name;
            }
            else
            {
                // parse error
                _state = state_data;
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

        default:
            // std::cerr << "what is this?" << std::endl;
            break;
        }

        ++_idx; // consume next char
    }

    return true;
}
