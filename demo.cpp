#include <iostream>
#include <fstream>
#include "html_lexer.hpp"
#include "stopwatch.hpp"

int main(int argc, char **argv)
{
    using namespace std;

    if (argc == 2)
    {
        std::ifstream file(argv[1]);
        if (file.is_open())
        {
            // read file content
            std::string html((std::istreambuf_iterator<char>(file)),
                             (std::istreambuf_iterator<char>()    ));
            file.close();

            stopwatch<double> timer("Tokenize HTML");

            timer.start();

            // tokenize
            html_lexer lexer(html);

            timer.stop();

            // print tokens
            lexer.print();

            // // test find_tag_by_class_names() and find_matching_tag()
            // size_t pos = 0;
            // while(true)
            // {
            //     pos = lexer.find_tag_by_class_names("ul","header-nav right", pos);
            //     if (pos != html_lexer::npos)
            //     {
            //         // test forward search
            //         size_t pos2 = lexer.find_matching_tag(pos);
            //         // test backward search
            //         size_t pos3 = lexer.find_matching_tag(pos2);
            //         // print original html
            //         size_t start = lexer.get_token(pos3)->get_start_position();
            //         size_t length = lexer.get_token(pos2)->get_end_position() - start;
            //         std::cerr << html.substr(start, length) << std::endl;
            //
            //         ++pos;
            //         continue;
            //     }
            //     break;
            // }

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
