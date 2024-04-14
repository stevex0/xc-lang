/// *==============================================================*
///  sourcefile.hpp
///
///  Contains the declaration of the SourceFile struct.
/// *==============================================================*
#ifndef SOURCEFILE_HPP
#define SOURCEFILE_HPP

#include "common.hpp"

namespace XC {

    struct SourceFile {
    public:
        std::string filename;
        std::vector<std::string> content;

        static std::unique_ptr<SourceFile> loadContent(const std::string filepath);
    };

}

#endif /* SOURCEFILE_HPP */