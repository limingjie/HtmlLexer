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
    _it             = _begin;
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
    text.clear();

    // iteration
    while (_it != _end)
    {
        if (isIterInsideTag)
        {
            if (text.size() > 0)
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
                text = ">";
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
                text = "=";
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
                    text = "Invalid HTML: No _end quote.";
                    _it = _end;
                }
                else
                {
                    htmlToken = HtmlTokenInTagQuotedString;
                    text = string(_it, findIt);
                    _it = findIt + 1;
                }

                break;
            }
            else if (*_it == '/')
            {
                // This is for tag self-closing, not _end tag slash.
                htmlToken = HtmlTokenTagSelfClosing;
                text = "/";
                ++_it;
                break;
            }
        }
        else
        {
            if (text.size() != 0 && *_it == '<')
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
                        text = string(start, _it);
                    }
                    else
                    {
                        htmlToken = HtmlTokenInvalid;
                        text = "Invalid HTML: Comment does not _end.";
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
                        text = string(start, _it);
                    }
                    else
                    {
                        htmlToken = HtmlTokenInvalid;
                        text = "Invalid HTML: Comment does not _end.";
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

                        text.push_back(*_it);
                        ++_it;
                    }

                    if (text.size() == 0)
                    {
                        htmlToken = HtmlTokenInvalid;
                        text = "Invalid HTML: Invalid tag name.";
                    }
                }

                break;
            }
        }

        // Put anything else into string.
        text.push_back(*_it);
        ++_it;
    }

    // token
    switch (htmlToken)
    {
    case HtmlTokenStartTag:
        isStartTag = true;
        isIterInsideTag = true;
        tagName = text;
        break;
    case HtmlTokenEndTag:
        isStartTag = false;
        isIterInsideTag = true;
        tagName = text;
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
                            text = string(_it, _begin + first);
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
                    text = "HTML Syntax Error: "
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
            text = "HTML Syntax Error: "
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
            text = "HTML Syntax Error: "
                   "Quoted string should appear after equal sign.";
        }
        break;
    case HtmlTokenText:
        first = text.find_first_not_of(Spaces);
        if (first == string::npos)
        {
            htmlToken = HtmlTokenNull;
        }
        else
        {
            last = text.find_last_not_of(Spaces);
            text = text.substr(first, last - first + 1);
        }
        break;
    case HtmlTokenInvalid:
        // error in text
        break;
    }

    lastHtmlToken = htmlToken;

    return htmlToken;
}
