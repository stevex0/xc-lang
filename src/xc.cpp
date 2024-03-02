/// *==============================================================*
///  xc.cpp
/// *==============================================================*
#include "include/xc.hpp"

namespace XC {

    void compile(const std::string target) {
        std::unique_ptr<Module> module = std::make_unique<Module>();

        module->source = SourceFile::loadContent(target).value_or(nullptr);
        if (module->source == nullptr) {
            exit(EXIT_FAILURE);
        }

        // TODO: Tokenization
        // TODO: Syntax Analysis
        // TODO: Semantic Analysis
        // TODO: Code Generation 
    }

}