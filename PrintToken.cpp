#include <iostream>
#include <fstream>
#include "html_lexer.hpp"
#include "Stopwatch.hpp"

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
            Stopwatch<double> time("Parsing HTML", true);
            html_lexer lexer(content);
            time.stop();
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
