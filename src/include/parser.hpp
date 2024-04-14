/// *==============================================================*
///  parser.hpp
///
///  Contains the declaration for the Parser class. 
/// *==============================================================*
#ifndef PARSER_HPP
#define PARSER_HPP

#include "common.hpp"
#include "xc.hpp"
#include "token.hpp"
#include "sourcefile.hpp"
#include "ast.hpp"

namespace XC {

    class Parser {
    public:
        Parser(const std::unique_ptr<Module>& module);

        static std::unique_ptr<Program> getProgramTree(const std::unique_ptr<Module>& module);
    private:
        const std::unique_ptr<Module>& module;

        uint32_t position;
        std::unique_ptr<Program> program;
        bool has_error;

        void parse(void);
        bool atEnd(void);

        const Token& current(void);
        const Token& next(void);
        const Token& peek(void);

        bool match(const TokenType expect);
        bool matchNext(const TokenType expect);

        bool consumeIf(const TokenType expect);

        AST* parseDeclaration(void);
        AST* parseFunction(void);
        AST* parseStructure(void);
        // TODO: AST* parseEnumerator(void);

        AST* parseStructureMembers(void);
        // TODO: AST* parseIdentifierList(void);
        AST* parseParameters(void);

        AST* parseBlockStatement(void);

        AST* parseStatement(void);
        AST* parseExpressionStatement(void);
        AST* parseVariableDeclarationStatement(void);
        AST* parseWhileIteration(void);
        AST* parseForIteration(void);
        AST* parseReturnStatement(void);
        AST* parseContinueStatement(void);
        AST* parseBreakStatement(void);
        AST* parseConditionalStatement(void);

        AST* parseDataType(void);
        AST* parseVariableDeclarator(void);

        AST* parseExpression(void);
        AST* parseAssignment(void);
        AST* parseBooleanOR(void);
        AST* parseBooleanXOR(void);
        AST* parseBooleanAND(void);
        AST* parseBitwiseOR(void);
        AST* parseBitwiseXOR(void);
        AST* parseBitwiseAND(void);
        AST* parseEquality(void);
        AST* parseRelational(void);
        AST* parseBitwiseShift(void);
        AST* parseAdditive(void);
        AST* parseMultiplicative(void);
        AST* parsePrefix(void);
        AST* parsePostfix(void);
        AST* parsePrimary(void);
        AST* parseLiteral(void);
        AST* parseNumberConstant(void);
        AST* parseIdentifierConstant(void);
        AST* parseGrouping(void);
        // TODO: AST* parseCastExpression(void);

        AST* parseExpressionList(void);

        ErrorNode* error(const std::string message);

        void reportError(ErrorNode*& error);

        using ProductionRule = AST* (XC::Parser::*)(void);

        AST* tryParse(const std::vector<ProductionRule>& rules, const std::string& error_message);
    };

}

#endif /* PARSER_HPP */