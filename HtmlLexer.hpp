#include <string>
#include <algorithm> // find()

class HtmlLexer
{
public:
    // Type declaration
    typedef int HtmlToken;

    // Invalid HTML
    static const HtmlToken HtmlTokenInvalid            = -1;
    // Null
    static const HtmlToken HtmlTokenNull               =  0;
    static const HtmlToken HtmlTokenStartTag           =  1; // "<" + TagName
    static const HtmlToken HtmlTokenEndTag             =  2; // "</" + TagName
    static const HtmlToken HtmlTokenTagEnd             =  3; // ">"
    static const HtmlToken HtmlTokenTagSelfClosing     =  4; // "/" of "/>"
    // HtmlTokenAttributeName is HtmlTokenInTagText which follows
    // HtmlTokenStartTag, HtmlTokenEndTag, HtmlTokenAttributeValue or
    // another HtmlTokenAttributeName (empty attribute)
    static const HtmlToken HtmlTokenAttributeName      =  5;
    static const HtmlToken HtmlTokenInTagEqualSign     =  6; // "="
    // HtmlTokenAttributeValue is HtmlTokenInTagSingleQuoted, TokenInTagText or
    // HtmlTokenInTagDoubledQuoted that follows HtmlTokenAttributeName and
    // HtmlTokenInTagEqualSign
    static const HtmlToken HtmlTokenAttributeValue     =  7;
    // single/double quoted string in tag
    static const HtmlToken HtmlTokenInTagQuotedString  =  8;
    static const HtmlToken HtmlTokenComment            =  9; // <!-- ... -->
    static const HtmlToken HtmlTokenCDATA              = 10; // <![CDATA[...]]>
    static const HtmlToken HtmlTokenRawText            = 11; // raw text
    static const HtmlToken HtmlTokenText               = 12; // normal text

    // HTML spaces characters are U+0020 SPACE, "tab" (U+0009), "LF" (U+000A),
    // "FF" (U+000C), and "CR" (U+000D)
    static const std::string Spaces;

    // HTML attribute name consists of chars other than space characters,
    // U+0000 NULL,U+0022 QUOTATION MARK ("), U+0027 APOSTROPHE ('),
    // ">" (U+003E), "/" (U+002F), and "=" (U+003D) characters
    static const std::string InvalidAttributeNameChars;

    // HTML attribute value consists of chars other than space characters,
    // U+0022 QUOTATION MARK characters ("), U+0027 APOSTROPHE characters ('),
    // "=" (U+003D) characters, "<" (U+003C) characters,
    // ">" (U+003E) characters, or U+0060 GRAVE ACCENT characters (`)
    static const std::string InvalidUnquotedAttributeValueChars;

private:
    bool                        isIterInsideTag;
    bool                        isStartTag;
    HtmlToken                   htmlToken;
    HtmlToken                   lastHtmlToken;
    std::string                 _html;
    std::string                 text;
    std::string                 tagName;
    std::string::const_iterator _begin;
    std::string::const_iterator _end;
    std::string::const_iterator _it;

    static inline bool isHtmlSpace(char c)
    {
        return (Spaces.find(c) != std::string::npos);
    }

    static inline bool isValidAttributeNameChar(char c)
    {
        return (InvalidAttributeNameChars.find(c) == std::string::npos);
    }

    static inline bool isValidUnquotedAttributeValueChar(char c)
    {
        return (InvalidUnquotedAttributeValueChars.find(c) == std::string::npos);
    }

public:
    HtmlLexer(const std::string &html);

    HtmlToken   getToken();
    std::string getTokenText()  {return text;}
    std::string getTagName()    {return tagName;}
    bool        end()           {return (_it == _end);}
};
