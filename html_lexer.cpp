#include "html_lexer.hpp"

    // enum state_type
    // {
    //     state_data,
    //     state_tag_open,
    //     state_end_tag_open,
    //     state_tag_name,
    //     state_self_closing_start_tag,
    //     state_before_attribute_name,
    //     state_attribute_name,
    //     state_after_attribute_name,
    //     state_before_attribute_value,
    //     state_attribute_value_unquoted,
    //     state_attribute_value_single_quoted,
    //     state_attribute_value_double_quoted,
    //     state_after_attribute_value_quoted
    // };

bool html_lexer::parse(std::string &html)
{
    state_type state = state_data;

    _html             = html; // Copy
    char   c;
    size_t idx        = 0;
    size_t size       = _html.size();
    html_token *token = nullptr;
    std::string temp_tag_name;

    while (idx < size)
    {
        c = _html[idx];

        switch (state)
        {
        case state_data:
			// std::cerr << "state_data" << std::endl;
            if (c == '<')
            {
                state = state_tag_open;
                // emit text token
                if (token != nullptr)
                {
                    token->set_end(idx);
                    tokens.push_back(token);
                    token = nullptr;
                }
            }
            else
            {
                // TODO: character token
                if (token == nullptr)
                {
                    token = new html_text_token();
                    token->set_start(idx);
                }

                ((html_text_token *)token)->append(c);
            }
            break;
        case state_tag_open:
			// std::cerr << "state_tag_open" << std::endl;
            if (c == '!')
            {
                if (html[idx + 1] == '-' && html[idx + 2] == '-')
                {
                    token = new html_comment_token();
                    token->set_start(idx - 1);

                    auto pos = _html.find("-->", idx + 3);
                    std::string comment;
                    if (pos == std::string::npos)
                    {
                        comment = _html.substr(idx + 3);
                        idx = size;
                    }
                    else
                    {
                        comment = _html.substr(idx + 3, pos - idx - 3);
                        idx = pos + 3;
                    }
                    ((html_comment_token *)token)->set_comment(comment);
                    token->set_end(idx);
                    tokens.push_back(token);
                    token = nullptr;
                    state = state_data;

                    continue;
                }
            }
            else if (c == '/')
            {
                state = state_end_tag_open;
            }
            else if (isupper(c))
            {
                token = new html_tag_token();
                token->set_start(idx);
                ((html_tag_token *)token)->append_to_name(tolower(c));
                state = state_tag_name;
            }
            else if (islower(c))
            {
                token = new html_tag_token();
                token->set_start(idx);
                ((html_tag_token *)token)->append_to_name(tolower(c));
                state = state_tag_name;
            }
            else
            {
                // Error
                state = state_data;
                c = '<';
                continue; // reconsume current char
            }
            break;
        case state_end_tag_open:
			// std::cerr << "state_end_tag_open" << std::endl;
            if (isupper(c))
            {
                token = new html_tag_token(false);
                token->set_start(idx);
                ((html_tag_token *)token)->append_to_name(tolower(c));
                state = state_tag_name;
            }
            else if (islower(c))
            {
                token = new html_tag_token(false);
                token->set_start(idx);
                ((html_tag_token *)token)->append_to_name(tolower(c));
                state = state_tag_name;
            }
            else
            {
                state = state_data;
            }
            break;
        case state_tag_name:
			// std::cerr << "state_tag_name" << std::endl;
            if (c == '\t' || c == '\r' || c == '\n' || c == ' ')
            {
                state = state_before_attribute_name;
            }
            else if (c == '/')
            {
                state = state_self_closing_start_tag;
            }
            else if (c == '>')
            {
                // check raw text
                temp_tag_name = ((html_tag_token *)token)->get_name();
                if (((html_tag_token *)token)->start_tag() &&
                    temp_tag_name == "script")

                {
                    state = state_raw_text;
                }
                else
                {
                    state = state_data;
                }
                // emit current tag token
                token->set_end(idx + 1);
                tokens.push_back(token);
                token = nullptr;
            }
            else if (isupper(c))
            {
                ((html_tag_token *)token)->append_to_name(tolower(c));
            }
            else
            {
                ((html_tag_token *)token)->append_to_name(c);
            }
            break;
        case state_before_attribute_name:
			// std::cerr << "state_before_attribute_name" << std::endl;
            if (c == '\t' || c == '\r' || c == '\n' || c == ' ')
            {
                // ignore
            }
            else if (c == '/')
            {
                state = state_self_closing_start_tag;
            }
            else if (c == '>')
            {
                // emit current tag token
                token->set_end(idx + 1);
                tokens.push_back(token);
                token = nullptr;
                state = state_data;
            }
            else if (isupper(c))
            {
                ((html_tag_token *)token)->append_to_attribute_name(tolower(c));
                state = state_attribute_name;
            }
            else
            {
                // error
                if (c == '"' || c == '\'' || c == '<' || c == '=')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)token)->append_to_attribute_name(c);
                state = state_attribute_name;
            }
            break;
        case state_attribute_name:
			// std::cerr << "state_attribute_name" << std::endl;
            if (c == '\t' || c == '\r' || c == '\n' || c == ' ')
            {
                state = state_after_attribute_name;
            }
            else if (c == '/')
            {
                ((html_tag_token *)token)->insert_attribute();
                state = state_self_closing_start_tag;
            }
            else if (c == '=')
            {
                state = state_before_attribute_value;
            }
            else if (c == '>')
            {
                // emit current tag token
                ((html_tag_token *)token)->insert_attribute();
                token->set_end(idx + 1);
                tokens.push_back(token);
                token = nullptr;
                state = state_data;
            }
            else if (isupper(c))
            {
                ((html_tag_token *)token)->append_to_attribute_name(tolower(c));
            }
            else
            {
                // error
                if (c == '"' || c == '\'' || c == '<')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)token)->append_to_attribute_name(c);
            }
            break;
        case state_after_attribute_name:
			// std::cerr << "state_after_attribute_name" << std::endl;
            if (c == '\t' || c == '\r' || c == '\n' || c == ' ')
            {
                // ignore
            }
            else if (c == '/')
            {
                ((html_tag_token *)token)->insert_attribute();
                state = state_self_closing_start_tag;
            }
            else if (c == '=')
            {
                state = state_before_attribute_value;
            }
            else if (c =='>')
            {
                // emit current tag token
                ((html_tag_token *)token)->insert_attribute();
                token->set_end(idx + 1);
                tokens.push_back(token);
                token = nullptr;
                state = state_data;
            }
            else if (isupper(c))
            {
                ((html_tag_token *)token)->append_to_attribute_name(tolower(c));
            }
            else
            {
                // error
                if (c == '"' || c == '\'' || c == '<')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)token)->append_to_attribute_name(c);
                state = state_attribute_name;
            }
            break;
        case state_before_attribute_value:
			// std::cerr << "state_before_attribute_value" << std::endl;
            if (c == '\t' || c == '\r' || c == '\n' || c == ' ')
            {
                // ignore
            }
            else if (c == '"')
            {
                state = state_attribute_value_double_quoted;
            }
            else if (c == '\'')
            {
                state = state_attribute_value_single_quoted;
            }
            else if (c == '>')
            {
                // emit current tag token
                ((html_tag_token *)token)->insert_attribute();
                token->set_end(idx + 1);
                tokens.push_back(token);
                token = nullptr;
                state = state_data;
            }
            else
            {
                // error
                if (c == '<' || c == '=' || c == '`')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)token)->append_to_attribute_value(c);
                state = state_attribute_value_unquoted;
            }
            break;
        case state_attribute_value_double_quoted:
			// std::cerr << "state_attribute_value_double_quoted" << std::endl;
            if (c == '"')
            {
                ((html_tag_token *)token)->insert_attribute();
                state = state_after_attribute_value_quoted;
            }
            else
            {
                ((html_tag_token *)token)->append_to_attribute_value(c);
            }
            break;
        case state_attribute_value_single_quoted:
			// std::cerr << "state_attribute_value_single_quoted" << std::endl;
            if (c == '\'')
            {
                ((html_tag_token *)token)->insert_attribute();
                state = state_after_attribute_value_quoted;
            }
            else
            {
                ((html_tag_token *)token)->append_to_attribute_value(c);
            }
            break;
        case state_attribute_value_unquoted:
			// std::cerr << "state_attribute_value_unquoted" << std::endl;
            if (c == '\t' || c == '\r' || c == '\n' || c == ' ')
            {
                ((html_tag_token *)token)->insert_attribute();
                state = state_before_attribute_name;
            }
            else if (c == '>')
            {
                // emit current tag token
                ((html_tag_token *)token)->insert_attribute();
                token->set_end(idx + 1);
                tokens.push_back(token);
                token = nullptr;
                state = state_data;
            }
            else
            {
                // error
                if (c == '"' || c == '\'' || c == '<' || c == '=' || c == '`')
                {
                    // but treat it as anything else
                }

                ((html_tag_token *)token)->append_to_attribute_value(c);
            }
            break;
        case state_after_attribute_value_quoted:
			// std::cerr << "state_after_attribute_value_quoted" << std::endl;
            if (c == '\t' || c == '\r' || c == '\n' || c == ' ')
            {
                state = state_before_attribute_name;
            }
            else if (c == '/')
            {
                state = state_self_closing_start_tag;
            }
            else if (c == '>')
            {
                // emit current tag token
                token->set_end(idx + 1);
                tokens.push_back(token);
                token = nullptr;
                state = state_data;
            }
            break;
        case state_self_closing_start_tag:
			// std::cerr << "state_self_closing_start_tag" << std::endl;
            if (c == '>')
            {
                ((html_tag_token *)token)->set_self_closing();
                // emit current tag token
                token->set_end(idx + 1);
                tokens.push_back(token);
                token = nullptr;
                state = state_data;
            }
            else
            {
                // error
                state = state_before_attribute_name;
                continue; // reconsume current char
            }
            break;
        case state_raw_text:
            {
                token = new html_text_token();
                token->set_start(idx);

                size_t first = idx;
                size_t last;
                size_t size_name = temp_tag_name.size() + 2;
                std::string raw_text;
                while (true)
                {
                    first = _html.find("</" + temp_tag_name, first);
                    std::cerr << first << std::endl;
                    if (first != std::string::npos)
                    {
                        last = first + size_name;
                        if (last < size)
                        {
                            c = _html[last];
                            if (c == '>' || c == '\t' || c == '\r' || c == '\n' || c == ' ')
                            {
                                if (first != idx)
                                {
                                    raw_text = _html.substr(idx, first - idx);
                                    idx = first;
                                }

                                break;
                            }
                            else
                            {
                                first = last;
                                continue;
                            }
                        }
                    }
                    else
                    {
                        raw_text = _html.substr(idx);
                        idx = size;
                    }

                    break;
                }

                if (raw_text.size() != 0)
                {
                    token->set_end(idx);
                    ((html_text_token *)token)->set_text(raw_text);
                    tokens.push_back(token);
                    token = nullptr;
                }

                state = state_data;

                continue; // outter loop
            }
            break;
        default:
            // std::cerr << "what is this?" << std::endl;
            break;
        }

        //++it; // consume next char
        ++idx; // consume next char
    }

    return true;
}
