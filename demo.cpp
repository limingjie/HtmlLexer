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
        }
        else
        {
            cerr << "Failed to open file: " << argv[1] << endl;
        }
    }
    else
    {
        cerr << "Usage: " << argv[0] << " filename.html" << endl;
    }

    return 0;
}
