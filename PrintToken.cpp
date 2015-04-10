#include <iostream>
#include <fstream>
#include "HtmlLexer.hpp"

void printHtmlToken(const std::string &html)
{
    using namespace std;

    HtmlLexer lexer(html);

    while (!lexer.end())
    {
        // analysis pass 2
        switch (lexer.getToken())
        {
        case HtmlLexer::HtmlTokenStartTag:
            cout << "[Start Tag      ] <" << lexer.getTokenText() << '\n';
            break;
        case HtmlLexer::HtmlTokenEndTag:
            cout << "[End Tag        ] </" << lexer.getTokenText() << '\n';
            break;
        case HtmlLexer::HtmlTokenTagEnd:
            cout << "[Tag End        ] >" << '\n';
            break;
        case HtmlLexer::HtmlTokenAttributeName:
            cout << "[Attribute Name ] " << lexer.getTokenText() << '\n';
            // TODO: add to attribute map
            break;
        case HtmlLexer::HtmlTokenInTagEqualSign:
            cout << "[Equal Sign     ] =" << '\n';
            break;
        case HtmlLexer::HtmlTokenAttributeValue:
            cout << "[Attribute Value] " << lexer.getTokenText() << '\n';
            // TODO: add to attribute map
            break;
        case HtmlLexer::HtmlTokenTagSelfClosing:
            cout << "[Self-closing   ] /" << '\n';
            break;
        case HtmlLexer::HtmlTokenRawText:
            // Tag end does not output if it is raw text element.
            cout << "[Tag End        ] >" << '\n';
            cout << "[Raw Text       ] " << lexer.getTokenText() << '\n';
            break;
        case HtmlLexer::HtmlTokenText:
            cout << "[Text           ] " << lexer.getTokenText() << '\n';
            break;
        case HtmlLexer::HtmlTokenComment:
            cout << "[Comment        ] " << lexer.getTokenText() << '\n';
            break;
        case HtmlLexer::HtmlTokenCDATA:
            cout << "[CDATA          ] " << lexer.getTokenText() << '\n';
            break;
        case HtmlLexer::HtmlTokenInvalid:
            cout << "[Error          ] " << lexer.getTokenText() << '\n';
            break;
        }
    }

    cout.flush();
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
