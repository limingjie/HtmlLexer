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
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()    ));
            file.close();

            stopwatch<double> timer("Parsing HTML");

            timer.start();

            // tokenize
            html_lexer lexer(content);

            timer.stop();

            // print tokens
            lexer.print();
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
