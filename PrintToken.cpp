#include <iostream>
#include <fstream>
#include "HtmlLexer.hpp"

void printHtmlToken(const std::string &html)
{
    using namespace std;

    HtmlLexer lexer(html);

    HtmlLexer::HtmlToken htmlToken;
    string data;
    string tagName;

    while (!lexer.ends())
    {
        htmlToken = lexer.getToken();
        data = lexer.getTokenValue();

        // analysis pass 2
        switch (htmlToken)
        {
        case HtmlLexer::HtmlTokenStartTag:
            cout << "[Start Tag      ] <" << lexer.getTagName() << endl;
            break;
        case HtmlLexer::HtmlTokenEndTag:
            cout << "[End Tag        ] </" << lexer.getTagName() << endl;
            break;
        case HtmlLexer::HtmlTokenTagEnd:
            cout << "[Tag End        ] >" << endl;
            break;
        case HtmlLexer::HtmlTokenAttributeName:
            cout << "[Attribute Name ] " << data << endl;
            // TODO: add to attribute map
            break;
        case HtmlLexer::HtmlTokenInTagEqualSign:
            cout << "[Equal Sign     ] =" << endl;
            break;
        case HtmlLexer::HtmlTokenAttributeValue:
            cout << "[Attribute Value] " << data << endl;
            // TODO: add to attribute map
            break;
        case HtmlLexer::HtmlTokenTagSelfClosing:
            cout << "[Self-closing   ] /" << endl;
            break;
        case HtmlLexer::HtmlTokenRawText:
            // Tag end does not output if it is raw text element.
            cout << "[Tag End        ] >" << endl;
            cout << "[Raw Text       ] " << data << endl;
            break;
        case HtmlLexer::HtmlTokenText:
            cout << "[Text           ] " << data << endl;
            break;
        case HtmlLexer::HtmlTokenComment:
            cout << "[Comment        ] " << data << endl;
            break;
        case HtmlLexer::HtmlTokenCDATA:
            cout << "[CDATA          ] " << data << endl;
            break;
        case HtmlLexer::HtmlTokenInvalid:
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
