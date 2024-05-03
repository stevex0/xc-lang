/// *==============================================================*
///  symboltable.hpp
/// *==============================================================*

#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP

#include "common.hpp"
#include "ast.hpp"

namespace XC {

    struct SymbolTable {
    public:
        std::unordered_map<std::string, Declaration*> symbols;

        SymbolTable(void);

        Declaration* lookup(const std::string& identifier);

        std::vector<Function*> getAllFunctions(void);
        std::vector<Structure*> getAllStructures(void);
        
        bool loadFunction(Function* function);
        bool loadStructure(Structure* structure);

        Function* lookupFunction(const std::string& identifier);
        Structure* lookupStructure(const std::string& identifier);
    };

    struct SymbolStack {
    public:
        std::vector<std::pair<const AST*, std::unordered_map<std::string, const DataType*>>> stack;

        SymbolStack(void);

        void pushStack(const AST* parent);

        void popStack(void);

        void addSymbol(const std::string& identifier, const DataType* type);

        const DataType* lookupSymbol(const std::string& identifier);
    };

}

#endif /* SYMBOLTABLE_HPP */