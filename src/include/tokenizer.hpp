/// *==============================================================*
///  tokenizer.hpp
///
///  Contains the declaration for the Tokenizer class. 
/// *==============================================================*
#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include "common.hpp"
#include "xc.hpp"
#include "token.hpp"
#include "sourcefile.hpp"

namespace XC {

    class Tokenizer {
    public:
        Tokenizer(const std::unique_ptr<Module>& module);

        std::optional<std::unique_ptr<TokenStream>> extractTokenStream();

    private:
        const std::unique_ptr<Module>& module;

        struct Position {
            uint32_t line;
            uint32_t column;
        } head, tail;

        std::string lexeme_buffer;
        std::unique_ptr<TokenStream> tokens;
        bool has_error;

        void tokenize(void);
        void process(const char c);
        const std::string& contentOnLine(const uint32_t line);
        bool atEnd(void);
        char current(void);
        char next(void);
        char peek(void);
        bool match(char expect);
        std::string consume(void);
        void addToken(const TokenType type);
        Token createToken(const TokenType type);

        TokenType error(const std::string message);
        TokenType error(const std::string message, const char symbol);

        TokenType extractSingleLineComment(void);
        TokenType extractMultiLineComment(void);
        TokenType extractIdentifier(void);
        TokenType extractWhitespace(void);
        TokenType extractZeroPrefixNumericLiteral(void);
        TokenType extractBinaryLiteral(void);
        TokenType extractOctalLiteral(void);
        TokenType extractHexadecimalLiteral(void);
        TokenType extractNumericLiteral(void);
        // TokenType extractCharacterLiteral(void);
        TokenType extractEscapeSequence(void);
        TokenType extractUnrecognizedSymbol(void);
    };

} // namespace XC

#endif /* TOKENIZER_HPP */