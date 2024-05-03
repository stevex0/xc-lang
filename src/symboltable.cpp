#include "include/symboltable.hpp"

using namespace XC;

SymbolTable::SymbolTable(void)
    : symbols(std::unordered_map<std::string, Declaration*>()) {}

Declaration* SymbolTable::lookup(const std::string& identifier) {
    if (symbols.count(identifier) <= 0) {
        return nullptr;
    }
    return symbols.at(identifier);
}

std::vector<Function*> SymbolTable::getAllFunctions(void) {
    std::vector<Function*> functions;
    for (const std::pair<std::string, Declaration*> kv : symbols) {
        if (Function* function = get_node_if(kv.second, Function)) {
            functions.push_back(function);
        }
    }
    return functions;
}

std::vector<Structure*> SymbolTable::getAllStructures(void) {
    std::vector<Structure*> structures;
    for (const std::pair<std::string, Declaration*> kv : symbols) {
        if (Structure* structure = get_node_if(kv.second, Structure)) {
            structures.push_back(structure);
        }
    }
    return structures;
}

bool SymbolTable::loadFunction(Function* function) {
    if (lookup(function->name->lexeme) == nullptr) {
        symbols.insert({function->name->lexeme, function});
        return true;
    }
    return false;
}

bool SymbolTable::loadStructure(Structure* structure) {
    if (lookup(structure->name->lexeme) == nullptr) {
        symbols.insert({structure->name->lexeme, structure});
        return true;
    }
    return false;
}

Function* SymbolTable::lookupFunction(const std::string& identifier) {
    return get_node_if(lookup(identifier), Function);
}

Structure* SymbolTable::lookupStructure(const std::string& identifier) {
    return get_node_if(lookup(identifier), Structure);
}

SymbolStack::SymbolStack(void)
    : stack(std::vector<std::pair<const AST*, std::unordered_map<std::string, const DataType*>>>()) {}

void SymbolStack::pushStack(const AST* parent) {
    stack.push_back(std::pair(parent, std::unordered_map<std::string, const DataType*>()));
}

void SymbolStack::popStack(void) {
    stack.pop_back();
}

void SymbolStack::addSymbol(const std::string& identifier, const DataType* type) {
    stack.back().second.insert({identifier, type});
}

const DataType* SymbolStack::lookupSymbol(const std::string& identifier) {
    for (const std::pair<const AST*, std::unordered_map<std::string, const DataType*>>& data : stack) {
        const std::unordered_map<std::string, const DataType*>& table = data.second;

        if (table.count(identifier) > 0) return {
            table.at(identifier)
        };
    }

    return nullptr;
}
