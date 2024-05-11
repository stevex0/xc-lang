/// *==============================================================*
///  xc.hpp
///
///  Contains declarations for the XC language.
/// *==============================================================*
#ifndef XC_HPP
#define XC_HPP

#include "common.hpp"
#include "sourcefile.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "symboltable.hpp"

namespace XC {

    struct Module {
    public:
        std::unique_ptr<SourceFile> source;
        std::unique_ptr<TokenStream> tokens;
        std::unique_ptr<Program> program;
        std::unique_ptr<SymbolTable> symbols;
        std::unique_ptr<SourceFile> code;
    };

    void compile(const std::string target);

}

#endif /* XC_HPP */