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
static const HtmlToken HtmlTokenInTagText          =   7;
static const HtmlToken HtmlTokenComment            =   8; // <!-- ... -->
static const HtmlToken HtmlTokenCDATA              =   9; // <![CDATA[ ... ]]>
static const HtmlToken HtmlTokenRawText            =  10; // raw text
static const HtmlToken HtmlTokenText               =  99; // normal text
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
static const std::string HtmlSpaces(" \r\n\t\x0c");
// HTML Attribute name consists of chars other than space characters,
// U+0000 NULL,U+0022 QUOTATION MARK ("), U+0027 APOSTROPHE ('), ">" (U+003E),
// "/" (U+002F), and "=" (U+003D) characters
static const std::string HtmlInvalidAttributeChar("> \r\n\t\x0c\"'/=");

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
    const HtmlToken lastHtmlToken,
    std::string &tagName,
    bool &isIterInsideTag,
    bool &isStartTag,
    std::string &data)
{
    using namespace std;

    auto      begin     = html.cbegin();
    auto      end       = html.cend();
    HtmlToken htmlToken = HtmlTokenNull;
    size_t    first;
    size_t    last;
    size_t    size;

    data.clear();

    // iteration
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
                // This is for tag self-closing, not end tag slash.
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
            first = size_t(it - begin);
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
                        if (first != size_t(it - begin))
                        {
                            data = string(it, begin + first);
                            htmlToken = HtmlTokenRawText;
                            it = begin + first;
                        }
                        // else
                        // {
                        //     // No raw text, keep it as HtmlTokenTagEnd
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
                           "Raw text elements does not end";
                    it = end;
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
            data = data.substr(first, last - first + 1);
        }
        break;
    case HtmlTokenInvalid:
        // error in data
        break;
    }

    return htmlToken;
}

void printHtmlToken(const std::string html)
{
    using namespace std;

    auto      end             = html.cend();
    auto      it              = html.cbegin();
    bool      isIterInsideTag = false;
    bool      isStartTag      = false;
    HtmlToken htmlToken       = HtmlTokenNull;
    string    data;
    string    tagName;

    while (it != end)
    {
        htmlToken = getHtmlToken(html, it, htmlToken, tagName, isIterInsideTag, isStartTag, data);

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
            cout << "[Attribute Name ] " << data << endl;
            // TODO: add to attribute map
            break;
        case HtmlTokenInTagEqualSign:
            cout << "[Equal Sign     ] =" << endl;
            break;
        case HtmlTokenAttributeValue:
            cout << "[Attribute Value] " << data << endl;
            // TODO: add to attribute map
            break;
        case HtmlTokenTagSelfClosing:
            cout << "[Self-closing   ] /" << endl;
            break;
        case HtmlTokenRawText:
            // Tag end does not output if it is raw text element.
            cout << "[Tag End        ] >" << endl;
            cout << "[Raw Text       ] " << data << endl;
            break;
        case HtmlTokenText:
            cout << "[Text           ] " << data << endl;
            break;
        case HtmlTokenComment:
            cout << "[Comment        ] " << data << endl;
            break;
        case HtmlTokenCDATA:
            cout << "[CDATA          ] " << data << endl;
            break;
        case HtmlTokenInvalid:
            cout << "[Error          ] " << data << endl;
            break;
        }
    }
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
            printHtmlToken(content);
        }
        else
        {
            cout << "Failed to open file: " << argv[1] << endl;
        }
    }
    else
    {
        cout << "Usage: " << argv[0] << " filename.html" << endl;
    }

    return 0;
}
