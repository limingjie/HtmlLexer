#include <iostream>
#include <fstream>
#include <string>
#include <algorithm> // find()

// HTML data Type
typedef int  HtmlToken;

// Invalid HTML
static const HtmlToken HtmlTokenInvalid            =  -1;
// Null
static const HtmlToken HtmlTokenNull               =   0;
/// [1...99] HTML tokens return by lexer getHtmlToken()
static const HtmlToken HtmlTokenStartTag           =   1; // "<" + TagName
static const HtmlToken HtmlTokenEndTag             =   2; // "</" + TagName
static const HtmlToken HtmlTokenTagEnd             =   3; // ">"
static const HtmlToken HtmlTokenTagSelfClosing     =   4; // "/" of "/>"
static const HtmlToken HtmlTokenInTagEqualSign     =   5; // "="
// single/double quoted string in tag
static const HtmlToken HtmlTokenInTagQuotedString  =   6;
// HtmlTokenAttributeName or HtmlTokenAttributeValue
static const HtmlToken HtmlTokenInTagText          =   8;
static const HtmlToken HtmlTokenComment            =   9; // <!-- ... -->
static const HtmlToken HtmlTokenCDATA              =  10; // <![CDATA[ ... ]]>
static const HtmlToken HtmlTokenText               =  99; // text between tags
/// [100...199] HTML tokens after further analysis base on lexer
// HtmlTokenAttributeName is HtmlTokenInTagText which follows HtmlTokenStartTag,
// HtmlTokenEndTag, HtmlTokenAttributeValue or
// another HtmlTokenAttributeName (empty attribute)
static const HtmlToken HtmlTokenAttributeName      = 101;
// HtmlTokenAttributeValue is HtmlTokenInTagSingleQuoted, TokenInTagText or
// HtmlTokenInTagDoubledQuoted that follows HtmlTokenAttributeName and
// HtmlTokenInTagEqualSign
static const HtmlToken HtmlTokenAttributeValue     = 102;

// HTML spaces characters are U+0020 SPACE, "tab" (U+0009), "LF" (U+000A),
// "FF" (U+000C), and "CR" (U+000D)
static std::string HtmlSpaces(" \r\n\t\x0c");
// HTML Attribute name consists of chars other than space characters,
// U+0000 NULL,U+0022 QUOTATION MARK ("), U+0027 APOSTROPHE ('), ">" (U+003E),
// "/" (U+002F), and "=" (U+003D) characters
static std::string HtmlInvalidAttributeChar("> \r\n\t\x0c\"'/=");

static inline bool isHtmlSpace(char c)
{
    return (HtmlSpaces.find(c) != std::string::npos);
}

static inline bool isValidHtmlAttributeChar(char c)
{
    return (HtmlInvalidAttributeChar.find(c) == std::string::npos);
}

static HtmlToken getHtmlToken(
    const std::string &html,
    std::string::const_iterator &it,
    const std::string::const_iterator &begin,
    const std::string::const_iterator &end,
    const bool isIterInsideTag,
    std::string &data // Output
    )
{
    using namespace std;

    HtmlToken htmlToken = HtmlTokenNull;

    data.clear();

    while (it != end)
    {
        if (isIterInsideTag)
        {
            if (data.size() > 0 && !isValidHtmlAttributeChar(*it))
            {
                htmlToken = HtmlTokenInTagText;
                break;
            }
            else if (*it == '>')
            {
                htmlToken = HtmlTokenTagEnd;
                data = ">";
                ++it;
                break;
            }
            else if (isHtmlSpace(*it))
            {
                ++it; // Omit spaces inside tag
                continue;
            }
            else if (*it == '=')
            {
                htmlToken = HtmlTokenInTagEqualSign;
                data = "=";
                ++it;
                break;
            }
            else if (*it == '\'' || *it == '"')
            {
                // Values delimited by single or double quotes
                auto findIt = it; // keep position
                ++it;
                findIt = find(it, end, *findIt);
                if (findIt == end)
                {
                    htmlToken = HtmlTokenInvalid;
                    data = "Invalid HTML: No end quote.";
                    it = end;
                }
                else
                {
                    htmlToken = HtmlTokenInTagQuotedString;
                    data = string(it, findIt);
                    it = findIt + 1;
                }

                break;
            }
            else if (*it == '/')
            {
                htmlToken = HtmlTokenTagSelfClosing;
                data = "/";
                ++it;
                break;
            }
        }
        else
        {
            if (data.size() != 0 && *it == '<')
            {
                htmlToken = HtmlTokenText;
                break;
            }
            else if (*it == '<')
            {
                // Check if it is CDATA or comment
                if (string(it + 1, it + 4) == "!--")
                {
                    size_t pos = html.find("-->", it - begin + 4);

                    if (pos != string::npos)
                    {
                        auto keepIt = it;
                        it = begin + pos + 3;
                        htmlToken = HtmlTokenComment;
                        data = string(keepIt, it);
                    }
                    else
                    {
                        htmlToken = HtmlTokenInvalid;
                        data = "Invalid HTML: Comment does not end.";
                        it = end;
                    }

                    break;
                }
                else if (string(it + 1, it + 9) == "![CDATA[")
                {
                    size_t pos = html.find("]]>", it - begin + 9);

                    if (pos != string::npos)
                    {
                        auto keepIt = it;
                        it = begin + pos + 3;
                        htmlToken = HtmlTokenCDATA;
                        data = string(keepIt, it);
                    }
                    else
                    {
                        htmlToken = HtmlTokenInvalid;
                        data = "Invalid HTML: Comment does not end.";
                        it = end;
                    }

                    break;
                }

                // Tag name
                ++it;
                if (it != end)
                {
                    if (*it == '/')
                    {
                        htmlToken = HtmlTokenEndTag;
                        ++it;
                    }
                    else
                    {
                        htmlToken = HtmlTokenStartTag;
                    }

                    // Get tag name
                    while (it != end)
                    {
                        if (*it == '>' || isHtmlSpace(*it) || *it == '/')
                        {
                            break;
                        }

                        data.push_back(*it);
                        ++it;
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
        data.push_back(*it);
        ++it;
    }

    return htmlToken;
}

int parseHtml(const std::string html)
{
    using namespace std;

    auto      begin           = html.cbegin();
    auto      end             = html.cend();
    auto      it              = html.cbegin();
    bool      isIterInsideTag = false;
    bool      isStartTag      = false;
    HtmlToken htmlToken       = HtmlTokenNull;
    HtmlToken lastHtmlToken   = HtmlTokenNull;
    size_t    first;
    size_t    last;
    size_t    size;
    string    attributeName;
    string    attributeValue;
    string    data;
    string    error;
    string    tagName;
    string    text;

    while (it != end)
    {
        htmlToken = getHtmlToken(html, it, begin, end, isIterInsideTag, data);

        // analysis pass 1
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
                first = it - begin;
                size = tagName.size() + 2;
                while (true)
                {
                    first = html.find("</" + tagName, first);
                    if (first != string::npos)
                    {
                        last = first + size;
                        if (last < html.size() &&
                            (html[last] == '>' || isHtmlSpace(html[last])))
                        {
                            text = string(it, begin + first);
                            htmlToken = HtmlTokenText;
                            it = begin + first;
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
                        error = "HTML Syntax Error: "
                                "Raw text elements does not end";
                        it = end;
                    }

                    break;
                }

            }
            break;
        case HtmlTokenInTagEqualSign:
            if (lastHtmlToken != HtmlTokenAttributeName)
            {
                htmlToken = HtmlTokenInvalid;
                error = "HTML Syntax Error: "
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
                error = "HTML Syntax Error: "
                        "Quoted string should appear after equal sign.";
            }
            break;
        case HtmlTokenInTagText:
            switch (lastHtmlToken)
            {
            case HtmlTokenStartTag:
            case HtmlTokenEndTag:
            case HtmlTokenAttributeName:
            case HtmlTokenAttributeValue:
                htmlToken = HtmlTokenAttributeName;
                break;
            case HtmlTokenInTagEqualSign:
                htmlToken = HtmlTokenAttributeValue;
                break;
            }
            break;
        case HtmlTokenText:
            first = data.find_first_not_of(HtmlSpaces);
            if (first == string::npos)
            {
                htmlToken = HtmlTokenNull;
            }
            else
            {
                last = data.find_last_not_of(HtmlSpaces);
                text = data.substr(first, last - first + 1);
            }
            break;
        case HtmlTokenInvalid:
            error = data;
            break;
        default:
            // what?
            break;
        }

        // analysis pass 2
        switch (htmlToken)
        {
        case HtmlTokenStartTag:
            cout << "[Start Tag      ] <" << tagName << endl;
            break;
        case HtmlTokenEndTag:
            cout << "[End Tag        ] </" << tagName << endl;
            break;
        case HtmlTokenTagEnd:
            cout << "[Tag End        ] >" << endl;
            break;
        case HtmlTokenAttributeName:
            attributeName = data;
            cout << "[Attribute Name ] " << attributeName << endl;
            // TODO: add to attribute map
            break;
        case HtmlTokenInTagEqualSign:
            cout << "[Equal Sign     ] =" << endl;
            break;
        case HtmlTokenAttributeValue:
            attributeValue = data;
            cout << "[Attribute Value] " << attributeValue << endl;
            // TODO: add to attribute map
            break;
        case HtmlTokenTagSelfClosing:
            cout << "[Self-closing   ] /" << endl;
            break;
        case HtmlTokenText:
            cout << "[Text           ] " << text << endl;
            break;
        case HtmlTokenComment:
            cout << "[Comment        ] " << data << endl;
            break;
        case HtmlTokenCDATA:
            cout << "[CDATA          ] " << data << endl;
            break;
        case HtmlTokenInvalid:
            cout << "[Error          ] " << error << endl;
            break;
        default:
            break;
        }

        lastHtmlToken = htmlToken;
    }

    return 0;
}

int main(int argc, char **argv)
{
    using namespace std;

    if (argc == 2)
    {
        std::ifstream file(argv[1]);
        if (file.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()    ));
            file.close();
            parseHtml(content);
        }
        else
        {
            cout << "Failed to open file: " << argv[1] << endl;
        }
    }
    else
    {
        cout << "Usage: partHtml filename.html" << endl;
    }

    return 0;
}
