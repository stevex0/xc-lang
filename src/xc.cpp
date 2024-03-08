/// *==============================================================*
///  xc.cpp
/// *==============================================================*
#include "include/xc.hpp"
#include "include/sourcefile.hpp"
#include "include/tokenizer.hpp"

using namespace XC;

void XC::compile(const std::string target) {
    std::unique_ptr<Module> module = std::make_unique<Module>();

    module->source = SourceFile::loadContent(target).value_or(nullptr);
    if (module->source == nullptr) {
        exit(EXIT_FAILURE);
    }

    module->tokens = Tokenizer(module).extractTokenStream().value_or(nullptr);
    if (module->tokens == nullptr) {
        exit(EXIT_FAILURE);
    }

    // TODO: Syntax Analysis
    // TODO: Semantic Analysis
    // TODO: Code Generation 
}