/// *==============================================================*
///  sourcefile.cpp
/// *==============================================================*
#include "include/sourcefile.hpp"

#include <cstring>
#include <fstream>

using namespace XC;

void SourceFile::writeOut(void) {
    std::ofstream outfile(filename);

    for (const std::string& line : content) {
        outfile << line << '\n';
    }

    outfile.flush();
    outfile.close();
}

std::unique_ptr<XC::SourceFile> XC::SourceFile::loadContent(const std::string filepath) {
    std::ifstream infile(filepath);

    if (!infile.is_open()) {
        std::cerr << "xc: \033[31merror\033[0m: " << std::strerror(errno) << ": `" << filepath << '`' << std::endl;
        return none();
    }

    std::unique_ptr<XC::SourceFile> source_file = std::make_unique<XC::SourceFile>();
    source_file->filename = filepath;

    std::string line;
    while (std::getline(infile, line)) {
        line.push_back('\n');
        source_file->content.push_back(line);
    }

    infile.close();

    return some(source_file);
}