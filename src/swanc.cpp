// Author: Manikandan M.
//
#include <iostream>

#include "compiler.h"

using namespace swan;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cout << "Usage: swanc [filename]\n";
        return -1;
    }

    std::string file_name_str = argv[1];
    fs::path file_path(file_name_str);
    fs::path search_path = file_path.parent_path();

    try {
        Compiler comp(search_path);
        file_path = fs::absolute(file_path);
        comp.compile(file_path, true);
    } catch (CompilerError e) {
        std::cout << "Error: " << e.message << '\n';
        return -1;
    }

    return 0;
}
