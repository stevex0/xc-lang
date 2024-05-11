/// *==============================================================*
///  analyzer.hpp
/// *==============================================================*

#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "common.hpp"
#include "xc.hpp"
#include "symboltable.hpp"

namespace XC {

    class Analyzer {
    public:
        Analyzer(const std::unique_ptr<Module>& module);

        static std::unique_ptr<SymbolTable> validateSemantics(const std::unique_ptr<Module>& module); 
    private:
        const std::unique_ptr<Module>& module;

        std::unique_ptr<SymbolTable> symbol_table;
        
        bool has_error;

        void checkSemantics(void);
        void loadSymbols(void);
        void validateStructures(void);
        void validateFunctions(void);

        void validateStructureMember(const Structure* structure);

        void validateFunctionOwner(const Function* function);
        void validateFunctionReturnType(const Function* function);
        void validateFunctionParameters(const Function* function);
        void validateFunctionBody(const Function* function);

        DataType* getTypeOfExpression(SymbolStack& symbols, Expression* expression);

        void validateBlockStatement(SymbolStack& stack, const BlockStatement* block);
        void validateStatement(SymbolStack& stack, const Statement* statement);

        DataType* getTypeOfPrefixExpression(SymbolStack& symbols, PrefixUnaryExpression* expression);
        DataType* getTypeOfPostfixExpression(SymbolStack& symbols, PostfixUnaryExpression* expression);
        DataType* getTypeOfBinaryExpression(SymbolStack& symbols, BinaryExpression* expression);
        DataType* getTypeOfNumberExpression(NumberConstant* number);
        DataType* getTypeOfIdentifier(SymbolStack& symbols, IdentifierConstant* identifier);
        DataType* getTypeOfLiteral(LiteralExpression* literal);
        DataType* getTypeOfMemberAccess(SymbolStack& symbols, MemberAccess* member_access);
        DataType* getTypeOfFunctionCall(SymbolStack& symbols, FunctionCall* function_call);

        DataType* copyDataType(const DataType* type);

        void validateConditionalStatement(SymbolStack& symbols, const ConditionalStatement* conditional);

        bool isIntegerType(const DataType* type);
        bool isFloatingPointType(const DataType* type);
        bool isBooleanType(const DataType* type);

        bool isSameType(const DataType* type_1, const DataType* type_2);

        bool withinLoop(SymbolStack& stack);
        const Function* getParentFunctionFromStack(SymbolStack& stack);

        void pushParametersToStack(SymbolStack& stack, const Function* function);

        std::nullptr_t error(const std::string& message, const Token* token);
    };

}

#endif /* ANALYZER_HPP */