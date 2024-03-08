/// *==============================================================*
///  token.hpp
///
///  Contains the declaration of the TokenType enum, Token struct,
///  and TokenStream.
/// *==============================================================*
#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "common.hpp"

namespace XC {

    enum class TokenType {
        UNKNOWN = -1,
        END_OF_FILE = 0,

        WHITESPACE,
        COMMENT,
        IDENTIFIER,

        INTEGER_LITERAL,
        FLOAT_LITERAL,
        CHARACTER_LITERAL,

        ARITHMETIC_OP_ADD,
        ARITHMETIC_OP_SUB,
        ARITHMETIC_OP_MUL,
        ARITHMETIC_OP_DIV,
        ARITHMETIC_OP_MOD,

        BITWISE_OP_AND,
        BITWISE_OP_OR,
        BITWISE_OP_XOR,
        BITWISE_OP_LEFT_SHIFT,
        BITWISE_OP_RIGHT_SHIFT,
        BITWISE_OP_COMPLEMENT,

        BOOLEAN_OP_AND,
        BOOLEAN_OP_OR,
        BOOLEAN_OP_XOR,
        BOOLEAN_OP_NOT,

        RELATIONAL_OP_EQUALITY,
        RELATIONAL_OP_INEQUALITY,
        RELATIONAL_OP_LESS_THAN,
        RELATIONAL_OP_GREATER_THAN,
        RELATIONAL_OP_LESS_THAN_EQUAL,
        RELATIONAL_OP_GREATER_THAN_EQUAL,

        ASSIGNMENT_ASSIGN,
        ASSIGNMENT_OP_ADD,
        ASSIGNMENT_OP_SUB,
        ASSIGNMENT_OP_MUL,
        ASSIGNMENT_OP_DIV,
        ASSIGNMENT_OP_MOD,
        ASSIGNMENT_OP_AND,
        ASSIGNMENT_OP_OR,
        ASSIGNMENT_OP_XOR,
        ASSIGNMENT_OP_LEFT_SHIFT,
        ASSIGNMENT_OP_RIGHT_SHIFT,

        OP_INCREMENT,
        OP_DECREMENT,

        PUNCTUATION_LEFT_PARENTHESIS,
        PUNCTUATION_RIGHT_PARENTHESIS,
        PUNCTUATION_LEFT_BRACKET,
        PUNCTUATION_RIGHT_BRACKET,
        PUNCTUATION_LEFT_BRACE,
        PUNCTUATION_RIGHT_BRACE,

        PUNCTUATION_SEMI_COLON,
        PUNCTUATION_COLON,
        PUNCTUATION_DOUBLE_COLON,
        PUNCTUATION_COMMA,
        PUNCTUATION_DOT,

        TYPE_VOID,
        TYPE_BOOL,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_BYTE,
        TYPE_SHORT,
        TYPE_INT,
        TYPE_LONG,

        LITERAL_BOOLEAN_TRUE,
        LITERAL_BOOLEAN_FALSE,
        LITERAL_REFERENCE_NULL,

        KEYWORD_RETURN,
        KEYWORD_BREAK,
        KEYWORD_CONTINUE,
        KEYWORD_IF,
        KEYWORD_ELSE,
        KEYWORD_WHILE,
        KEYWORD_FOR,
        KEYWORD_STRUCT,
        KEYWORD_ENUM
    };

    struct Token {
    public:
        uint32_t index;
        uint32_t line;
        uint32_t column;

        TokenType type;
        std::string lexeme;
    };

    using TokenStream = std::vector<Token>;

}

#endif /* TOKEN_HPP */