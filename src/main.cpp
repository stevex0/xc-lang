/// *==============================================================*
///  main.cpp
/// *==============================================================*
#include "include/xc.hpp"

using namespace XC;

int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cerr << "usage:\n\txc [TARGET]" << std::endl;
        exit(EXIT_FAILURE);
    }

    compile(std::string(argv[1]));

    return EXIT_SUCCESS;
}