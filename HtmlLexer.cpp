#include "HtmlLexer.hpp"

const std::string HtmlLexer::Spaces(" \r\n\t\x0c");
const std::string HtmlLexer::InvalidAttributeNameChars("> \r\n\t\x0c\"'/=");
const std::string HtmlLexer::InvalidUnquotedAttributeValueChars
                      ("> \r\n\t\x0c\"'=<`");

HtmlLexer::HtmlLexer(const std::string &html)
{
    _html           = html;
    _begin          = _html.cbegin();
    _end            = _html.cend();
    _it             = _html.cbegin();
    isIterInsideTag = false;
    isStartTag      = false;
    htmlToken       = HtmlTokenNull;
    lastHtmlToken   = HtmlTokenNull;
}

HtmlLexer::HtmlToken HtmlLexer::getToken()
{
    using namespace std;

    size_t first;
    size_t last;
    size_t pos;
    size_t size;

    htmlToken = HtmlTokenNull;
    data.clear();

    // iteration
    while (_it != _end)
    {
        if (isIterInsideTag)
        {
            if (data.size() > 0)
            {
                if (lastHtmlToken == HtmlTokenInTagEqualSign
                    && !isValidUnquotedAttributeValueChar(*_it))
                {
                    htmlToken = HtmlTokenAttributeValue;
                    break;
                }
                else if ((lastHtmlToken == HtmlTokenStartTag
                          || lastHtmlToken == HtmlTokenEndTag
                          || lastHtmlToken == HtmlTokenAttributeName
                          || lastHtmlToken == HtmlTokenAttributeValue )
                         && !isValidAttributeNameChar(*_it))
                {
                    htmlToken = HtmlTokenAttributeName;
                    break;
                }
            }
            else if (*_it == '>')
            {
                htmlToken = HtmlTokenTagEnd;
                data = ">";
                ++_it;
                break;
            }
            else if (isHtmlSpace(*_it))
            {
                ++_it; // Omit spaces inside tag
                continue;
            }
            else if (*_it == '=')
            {
                htmlToken = HtmlTokenInTagEqualSign;
                data = "=";
                ++_it;
                break;
            }
            else if (*_it == '\'' || *_it == '"')
            {
                // Values delimited by single or double quotes
                auto findIt = _it; // keep position
                ++_it;
                findIt = find(_it, _end, *findIt);
                if (findIt == _end)
                {
                    htmlToken = HtmlTokenInvalid;
                    data = "Invalid HTML: No _end quote.";
                    _it = _end;
                }
                else
                {
                    htmlToken = HtmlTokenInTagQuotedString;
                    data = string(_it, findIt);
                    _it = findIt + 1;
                }

                break;
            }
            else if (*_it == '/')
            {
                // This is for tag self-closing, not _end tag slash.
                htmlToken = HtmlTokenTagSelfClosing;
                data = "/";
                ++_it;
                break;
            }
        }
        else
        {
            if (data.size() != 0 && *_it == '<')
            {
                htmlToken = HtmlTokenText;
                break;
            }
            else if (*_it == '<')
            {
                // Check if _it is CDATA or comment
                if (string(_it + 1, _it + 4) == "!--")
                {
                    pos = _html.find("-->", _it - _begin + 4);

                    if (pos != string::npos)
                    {
                        auto start = _it;
                        _it = _begin + pos + 3;
                        htmlToken = HtmlTokenComment;
                        data = string(start, _it);
                    }
                    else
                    {
                        htmlToken = HtmlTokenInvalid;
                        data = "Invalid HTML: Comment does not _end.";
                        _it = _end;
                    }

                    break;
                }
                else if (string(_it + 1, _it + 9) == "![CDATA[")
                {
                    pos = _html.find("]]>", _it - _begin + 9);

                    if (pos != string::npos)
                    {
                        auto start = _it;
                        _it = _begin + pos + 3;
                        htmlToken = HtmlTokenCDATA;
                        data = string(start, _it);
                    }
                    else
                    {
                        htmlToken = HtmlTokenInvalid;
                        data = "Invalid HTML: Comment does not _end.";
                        _it = _end;
                    }

                    break;
                }

                // Tag name
                ++_it;
                if (_it != _end)
                {
                    if (*_it == '/')
                    {
                        htmlToken = HtmlTokenEndTag;
                        ++_it;
                    }
                    else
                    {
                        htmlToken = HtmlTokenStartTag;
                    }

                    // Get tag name
                    while (_it != _end)
                    {
                        if (*_it == '>' || isHtmlSpace(*_it) || *_it == '/')
                        {
                            break;
                        }

                        data.push_back(*_it);
                        ++_it;
                    }

                    if (data.size() == 0)
                    {
                        htmlToken = HtmlTokenInvalid;
                        data = "Invalid HTML: Invalid tag name.";
                    }
                }

                break;
            }
        }

        // Put anything else into string.
        data.push_back(*_it);
        ++_it;
    }

    // token
    switch (htmlToken)
    {
    case HtmlTokenStartTag:
        isStartTag = true;
        isIterInsideTag = true;
        tagName = data;
        break;
    case HtmlTokenEndTag:
        isStartTag = false;
        isIterInsideTag = true;
        tagName = data;
        break;
    case HtmlTokenTagSelfClosing:
        isStartTag = false;
        break;
    case HtmlTokenTagEnd:
        isIterInsideTag = false;
        // Raw text elements, <script>, <style>
        // Escapable raw text elements, <textarea>, <title>
        if (isStartTag &&
            (tagName == "script" || tagName == "style" ||
             tagName == "textarea" || tagName == "title" ))
        {
            first = size_t(_it - _begin);
            size = tagName.size() + 2;
            while (true)
            {
                first = _html.find("</" + tagName, first);
                if (first != string::npos)
                {
                    last = first + size;
                    if (last < _html.size() &&
                        (_html[last] == '>' || isHtmlSpace(_html[last])))
                    {
                        if (first != size_t(_it - _begin))
                        {
                            data = string(_it, _begin + first);
                            htmlToken = HtmlTokenRawText;
                            _it = _begin + first;
                        }
                        // else
                        // {
                        //     // No raw text, keep _it as HtmlTokenTagEnd
                        // }

                        break;
                    }
                    else
                    {
                        first = last;
                        continue;
                    }
                }
                else
                {
                    htmlToken = HtmlTokenInvalid;
                    data = "HTML Syntax Error: "
                           "Raw text elements does not _end";
                    _it = _end;
                }

                break;
            }

        }
        else
        {
            tagName.clear();
        }
        break;
    case HtmlTokenInTagEqualSign:
        if (lastHtmlToken != HtmlTokenAttributeName)
        {
            htmlToken = HtmlTokenInvalid;
            data = "HTML Syntax Error: "
                   "No attribute name before equal sign";
        }
        break;
    case HtmlTokenInTagQuotedString:
        if (lastHtmlToken == HtmlTokenInTagEqualSign)
        {
            htmlToken = HtmlTokenAttributeValue;
        }
        else
        {
            htmlToken = HtmlTokenInvalid;
            data = "HTML Syntax Error: "
                   "Quoted string should appear after equal sign.";
        }
        break;
    case HtmlTokenText:
        first = data.find_first_not_of(Spaces);
        if (first == string::npos)
        {
            htmlToken = HtmlTokenNull;
        }
        else
        {
            last = data.find_last_not_of(Spaces);
            data = data.substr(first, last - first + 1);
        }
        break;
    case HtmlTokenInvalid:
        // error in data
        break;
    }

    lastHtmlToken = htmlToken;

    return htmlToken;
}
