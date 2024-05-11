/// *==============================================================*
///  ast.hpp
///
///  Contains the declarations for the Abstract Syntax Tree. 
/// *==============================================================*
#ifndef AST_HPP
#define AST_HPP

#include "common.hpp"
#include "token.hpp"

namespace XC {

    struct AST;
    struct ErrorNode;
    struct Program;
    struct Declaration;
    struct Function;
    struct Structure;
    struct Enumerator;
    struct DataType;
    struct BasicType;
    struct StructureMembers;
    struct IdentifierList;
    struct ParameterList;
    struct VariableDeclarator;
    struct BlockStatement;
    struct Statement;
    struct VariableDeclarationStatement;
    struct ExpressionStatement;
    struct ConditionalStatement;
    struct ReturnStatement;
    struct ContinueStatement;
    struct BreakStatement;
    struct WhileIteration;
    struct ForIteration;
    struct Expression;
    struct ExpressionList;
    struct PrefixUnaryExpression;
    struct PostfixUnaryExpression;
    struct BinaryExpression;
    struct LiteralExpression;
    struct NumberConstant;
    struct IdentifierConstant;
    struct CastExpression;
    struct ArrayInitializerList;
    struct ArrayDeclaration;
    struct MemberAccess;
    struct FunctionCall;
    struct ArrayAccess;

    enum class ASTType {
        AST,
        ErrorNode,
        Program,
        Declaration,
        Function,
        Structure,
        Enumerator,
        DataType,
        BasicType,
        StructureMembers,
        IdentifierList,
        ParameterList,
        VariableDeclarator,
        BlockStatement,
        Statement,
        VariableDeclarationStatement,
        ExpressionStatement,
        ConditionalStatement,
        ReturnStatement,
        ContinueStatement,
        BreakStatement,
        WhileIteration,
        ForIteration,
        Expression,
        ExpressionList,
        PrefixUnaryExpression,
        PostfixUnaryExpression,
        BinaryExpression,
        LiteralExpression,
        NumberConstant,
        IdentifierConstant,
        CastExpression,
        ArrayInitializerList,
        ArrayDeclaration,
        MemberAccess,
        FunctionCall,
        ArrayAccess
    };

    using IdentifierToken = const Token;
    using OperatorToken = const Token;
    using NumericToken = const Token;
    using LiteralValueToken = const Token;

    // <*> ================================================================ <*>

    struct AST {
    public:
        virtual ~AST() = default;
        virtual ASTType type(void) const = 0;
    };

    struct Declaration : public AST {
    public:
        ASTType type(void) const {
            return ASTType::Declaration;
        }
    };

    struct Statement : public AST {
    public:
        ASTType type(void) const {
            return ASTType::Statement;
        }
    };

    struct DataType : public AST {
    public:
        bool is_reference;
        IdentifierToken* type_name;
        uint32_t dimensions;

        DataType(void)
            : is_reference(false),
              type_name(nullptr),
              dimensions(0) {}

        ASTType type(void) const {
            return ASTType::DataType;
        }
    };

    struct Expression : public AST {
    public:
        DataType* evaluated_type;

        Expression(void)
            : evaluated_type(nullptr) {}

        ~Expression() {
            delete evaluated_type;
        }

        ASTType type(void) const {
            return ASTType::Expression;
        }
    };

    // <*> ================================================================ <*>

    struct ErrorNode : public AST {
    public:
        std::string reason;
        uint32_t occurrence;
        ErrorNode* additional_errors;

        ErrorNode(void)
            : reason(std::string()),
              occurrence(0),
              additional_errors(nullptr) {}

        ~ErrorNode() {
            delete additional_errors;
        }

        void appendError(ErrorNode* error) {
            if (additional_errors != nullptr) {
                ErrorNode* current = additional_errors;

                while (current->additional_errors != nullptr) {
                    current = current->additional_errors;
                }

                current->additional_errors = error;
            } else {
                additional_errors = error;
            }
        }

        ASTType type(void) const {
            return ASTType::ErrorNode;
        }
    };

    // <*> ================================================================ <*>

    // struct DataType : public AST {
    // public:
    //     bool is_reference;
    //     IdentifierToken* type_name;
    //     uint32_t dimensions;

    //     DataType(void)
    //         : is_reference(false),
    //           type_name(nullptr),
    //           dimensions(0) {}

    //     ASTType type(void) const {
    //         return ASTType::DataType;
    //     }
    // };

    struct VariableDeclarator : public AST {
    public:
        DataType* data_type;
        IdentifierToken* variable_name;

        VariableDeclarator(void)
            : data_type(nullptr),
              variable_name(nullptr) {}

        ~VariableDeclarator() {
            delete data_type; 
        }

        ASTType type(void) const {
            return ASTType::VariableDeclarator;
        }
    };

    struct StructureMembers : public AST {
    public:
        std::vector<VariableDeclarator*> members;

        StructureMembers(void)
            : members(std::vector<VariableDeclarator*>()){}

        ~StructureMembers() {
            for (VariableDeclarator*& member : members) {
                delete member;
                member = nullptr;
            }
        }

        ASTType type(void) const {
            return ASTType::StructureMembers;
        }
    };

    struct ParameterList : public AST {
    public:
        std::vector<VariableDeclarator*> parameters;

        ParameterList(void)
            : parameters(std::vector<VariableDeclarator*>()) {}

        ~ParameterList() {
            for (VariableDeclarator*& declarator : parameters) {
                delete declarator;
                declarator = nullptr;
            }
        }

        ASTType type(void) const {
            return ASTType::ParameterList;
        }
    };

    struct BlockStatement : public AST {
    public:
        std::vector<Statement*> statements;

        BlockStatement(void)
            : statements(std::vector<Statement*>()) {}


        ~BlockStatement() {
            for (Statement*& statement : statements) {
                delete statement;
                statement = nullptr;
            }
        }

        ASTType type(void) const {
            return ASTType::BlockStatement;
        }
    };

    // <*> ================================================================ <*>

    struct Program : public AST {
    public:
        std::vector<Declaration*> declarations;

        Program(void)
            : declarations(std::vector<Declaration*>()) {}

        ~Program() {
            for (Declaration*& declaration : declarations) {
                delete declaration;
                declaration = nullptr;
            }
        }

        ASTType type(void) const {
            return ASTType::Program;
        }
    };

    struct Function : public Declaration {
    public:
        IdentifierToken* owner; // nullptr -> no owner
        DataType* return_type;
        IdentifierToken* name;
        ParameterList* parameters;
        BlockStatement* body;

        Function(void)
            : owner(nullptr),
              return_type(nullptr),
              name(nullptr),
              parameters(nullptr),
              body(nullptr) {}

        ~Function() {
            delete return_type;
            delete parameters;
            delete body;
        }

        ASTType type(void) const {
            return ASTType::Function;
        }
    };

    struct Structure : public Declaration {
    public:
        IdentifierToken* name;
        StructureMembers* members;

        Structure(void)
            : name(nullptr),
              members(nullptr) {}

        ~Structure() {
            delete members;
        }

        ASTType type(void) const {
            return ASTType::Structure;
        }
    };

    // <*> ================================================================ <*>

    struct ExpressionStatement : public Statement {
    public:
        Expression* expression;

        ExpressionStatement(void)
            : expression(nullptr) {}

        ~ExpressionStatement() {
            delete expression;
        }

        ASTType type(void) const {
            return ASTType::ExpressionStatement;
        }
    };

    struct ConditionalStatement : public Statement {
    public:
        Expression* condition;
        BlockStatement* body;
        ConditionalStatement* else_case;

        ConditionalStatement(void)
            : condition(nullptr),
              body(nullptr),
              else_case(nullptr) {}

        ~ConditionalStatement() {
            delete condition;
            delete body;
            delete else_case;
        } 

        ASTType type(void) const {
            return ASTType::ConditionalStatement;
        }
    };

    struct VariableDeclarationStatement : public Statement {
    public:
        VariableDeclarator* declarator;
        Expression* initial;

        VariableDeclarationStatement(void)
            : declarator(nullptr),
              initial(nullptr) {}

        ~VariableDeclarationStatement() {
            delete declarator;
            delete initial;
        }

        ASTType type(void) const {
            return ASTType::VariableDeclarationStatement;
        }
    };

    struct WhileIteration : public Statement {
    public:
        Expression* condition;
        BlockStatement* body;

        WhileIteration(void)
            : condition(nullptr),
              body(nullptr) {}

        ASTType type(void) const {
            return ASTType::WhileIteration;
        }
    };

    struct ForIteration : public Statement {
    public:
        VariableDeclarationStatement* initial;
        Expression* condition;
        Expression* update;
        BlockStatement* body;

        ForIteration(void)
            : initial(nullptr),
              condition(nullptr),
              update(nullptr),
              body(nullptr) {}

        ~ForIteration() {
            delete initial;
            delete condition;
            delete update;
            delete body;
        }

        ASTType type(void) const {
            return ASTType::ForIteration;
        }
    };

    struct ReturnStatement : public Statement {
    public:
        Expression* expression;

        ReturnStatement(void)
            : expression(nullptr) {}

        ~ReturnStatement() {
            delete expression;
        }

        ASTType type(void) const {
            return ASTType::ReturnStatement;
        }
    };

    struct ContinueStatement : public Statement {
    public:
        ASTType type(void) const {
            return ASTType::ContinueStatement;
        }
    };

    struct BreakStatement : public Statement {
    public:
        ASTType type(void) const {
            return ASTType::BreakStatement;
        }
    };

    // <*> ================================================================ <*>

    struct ExpressionList : public AST {
    public:
        std::vector<Expression*> expressions;

        ExpressionList(void)
            : expressions(std::vector<Expression*>()) {}

        ~ExpressionList() {
            for (Expression*& expression : expressions) {
                delete expression;
                expression = nullptr;
            }
        }

        ASTType type(void) const {
            return ASTType::ExpressionList;
        }
    };

    struct PrefixUnaryExpression : public Expression {
    public:
        OperatorToken* operation;
        Expression* operand;

        PrefixUnaryExpression(void)
            : operation(nullptr),
              operand(nullptr) {}

        ~PrefixUnaryExpression() {
            delete operand;
        }

        ASTType type(void) const {
            return ASTType::PrefixUnaryExpression;
        }
    };

    struct PostfixUnaryExpression : public Expression {
    public:
        OperatorToken* operation;
        Expression* operand;

        PostfixUnaryExpression(void)
            : operation(nullptr),
              operand(nullptr) {}

        ~PostfixUnaryExpression() {
            delete operand;
        }

        ASTType type(void) const {
            return ASTType::PostfixUnaryExpression;
        }
    };

    struct BinaryExpression : public Expression {
    public:
        OperatorToken* operation;
        Expression* left_operand;
        Expression* right_operand;

        BinaryExpression(void)
            : operation(nullptr),
              left_operand(nullptr),
              right_operand(nullptr) {}

        ~BinaryExpression() {
            delete left_operand;
            delete right_operand;
        }

        ASTType type(void) const {
            return ASTType::BinaryExpression;
        }
    };

    // <*> ================================================================ <*>

    struct LiteralExpression : Expression {
    public:
        LiteralValueToken* value;

        LiteralExpression(void)
            : value(nullptr) {}

        ASTType type(void) const {
            return ASTType::LiteralExpression;
        }
    };

    struct NumberConstant : Expression {
    public:
        NumericToken* value;

        NumberConstant(void)
            : value(nullptr) {}

        ASTType type(void) const {
            return ASTType::NumberConstant;
        }
    };

    struct IdentifierConstant : Expression {
    public:
        IdentifierToken* value;

        IdentifierConstant(void)
            : value(nullptr) {}

        ASTType type(void) const {
            return ASTType::IdentifierConstant;
        }
    };

    struct MemberAccess : public Expression {
    public:
        Expression* owner;
        IdentifierToken* member;

        MemberAccess(void)
            : owner(nullptr),
              member(nullptr) {}

        ~MemberAccess() {
            delete owner;
        }

        ASTType type(void) const {
            return ASTType::MemberAccess;
        }
    };

    struct FunctionCall : public Expression {
    public:
        Expression* function;
        ExpressionList* arguments;

        FunctionCall(void)
            : function(nullptr),
              arguments(nullptr) {}

        ~FunctionCall() {
            delete function;
            delete arguments;
        }

        ASTType type(void) const {
            return ASTType::FunctionCall;
        }
    };

    struct ArrayAccess : public Expression {
    public:
        Expression* array;
        Expression* index;

        ArrayAccess(void)
            : array(nullptr),
              index(nullptr) {}

        ~ArrayAccess() {
            delete array;
            delete index;
        }

        ASTType type(void) const {
            return ASTType::ArrayAccess;
        }
    };

    struct CastExpression : public Expression {
    public:
        DataType* data_type;
        Expression* expression;

        CastExpression(void)
            : data_type(nullptr),
              expression(nullptr) {}

        ~CastExpression() {
            delete data_type;
            delete expression;
        }

        ASTType type(void) const {
            return ASTType::CastExpression;
        }
    };

    // <*> ================================================================ <*>

    Expression* newBinaryExpression(OperatorToken* _operator, Expression* left_operand, Expression* right_operand);
    Expression* newPrefixExpression(OperatorToken* _operator, Expression* operand);
    Expression* newPostfixExpression(OperatorToken* _operator, Expression* operand);

    // <*> ================================================================ <*>

    #define get_node_if(node, is) ((node_is(node, is)) ? (is*) (node) : nullptr)
    #define node_is(node, is) (node != nullptr && node->type() == ASTType::is)

    // For debugging purposes
    void printTree(AST* node, std::string indent, bool last);
}

#endif /* AST_HPP */