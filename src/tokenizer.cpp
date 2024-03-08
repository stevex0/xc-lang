/// *==============================================================*
///  tokenizer.cpp
/// *==============================================================*
#include "include/tokenizer.hpp"
#include "include/cclass.hpp"

using namespace XC;

static const std::unordered_map<std::string, TokenType> reserved_words = {
    {   "bool",        TokenType::TYPE_BOOL                },
    {   "break",       TokenType::KEYWORD_BREAK            },
    {   "byte",        TokenType::TYPE_BYTE                },
    {   "continue",    TokenType::KEYWORD_CONTINUE         },
    {   "else",        TokenType::KEYWORD_ELSE             },
    {   "enum",        TokenType::KEYWORD_ENUM             },
    {   "false",       TokenType::LITERAL_BOOLEAN_FALSE    },
    {   "float",       TokenType::TYPE_FLOAT               },
    {   "for",         TokenType::KEYWORD_FOR              },
    {   "if",          TokenType::KEYWORD_IF               },
    {   "int",         TokenType::TYPE_INT                 },
    {   "long",        TokenType::TYPE_LONG                },
    {   "null",        TokenType::LITERAL_REFERENCE_NULL   },
    {   "return",      TokenType::KEYWORD_RETURN           },
    {   "short",       TokenType::TYPE_SHORT               },
    {   "struct",      TokenType::KEYWORD_STRUCT           },
    {   "true",        TokenType::LITERAL_BOOLEAN_TRUE     },
    {   "void",        TokenType::TYPE_VOID                },
    {   "while",       TokenType::KEYWORD_WHILE            }
};

Tokenizer::Tokenizer(const std::unique_ptr<Module>& module)
    : module(module),
      head({0, 0}),
      tail({0, 0}),
      lexeme_buffer(),
      tokens(std::make_unique<TokenStream>()),
      has_error(false) {
    tokenize();
}

void Tokenizer::tokenize(void) {
    while (!atEnd()) {
        process(next());
    }

    addToken(TokenType::END_OF_FILE);
}

void Tokenizer::process(const char c) {
    switch (c) {
        case '(':
            // --> ('(') --> Left parenthesis ('(')
            addToken(TokenType::PUNCTUATION_LEFT_PARENTHESIS);
            break;
        case ')':
            // --> (')') --> Right parenthesis (')')
            addToken(TokenType::PUNCTUATION_RIGHT_PARENTHESIS);
            break;
        case '[':
            // --> ('[') --> Left bracket ('[')
            addToken(TokenType::PUNCTUATION_LEFT_BRACKET);
            break;
        case ']':
            // --> (']') --> Right bracket (']')
            addToken(TokenType::PUNCTUATION_RIGHT_BRACKET);
            break;
        case '{':
            // --> ('{') --> Left brace ('{')
            addToken(TokenType::PUNCTUATION_LEFT_BRACE);
            break;
        case '}':
            // --> ('}') --> Right brace ('}')
            addToken(TokenType::PUNCTUATION_RIGHT_BRACE);
            break;
        case ';':
            // --> (';') --> Semi colon (';')
            addToken(TokenType::PUNCTUATION_SEMI_COLON);
            break;
        case ':':
            // --> (':')
            //       |
            //       +--- (':') --> Double colon ('::')
            //       |
            //       +--> Colon (':')
            addToken(match(':') ? TokenType::PUNCTUATION_DOUBLE_COLON : TokenType::PUNCTUATION_COLON);
            break;
        case ',':
            // --> (',') --> Comma (',')
            addToken(TokenType::PUNCTUATION_COMMA);
            break;
        case '.':
            // --> ('.') --> Dot ('.')
            addToken(TokenType::PUNCTUATION_DOT);
            break;
        case '~':
            // --> ('~') --> Bitwise complement ('~')
            addToken(TokenType::BITWISE_OP_COMPLEMENT);
            break;
        case '=':
            // --> ('=')
            //       |
            //       +--- ('=') --> Relational equality ('==')
            //       |
            //       +--> Assignment ('=')
            addToken(match('=') ? TokenType::RELATIONAL_OP_EQUALITY : TokenType::ASSIGNMENT_ASSIGN);
            break;
        case '+':
            // --> ('+')
            //       |
            //       +--- ('+') --> Increment ('++')
            //       |
            //       +--- ('=') --> Assignment addition ('+=')
            //       |
            //       +--> Arithmetic addition ('+')
            addToken(match('+') ? TokenType::OP_INCREMENT : (match('=') ? TokenType::ASSIGNMENT_OP_ADD : TokenType::ARITHMETIC_OP_ADD));
            break;
        case '-':
            // --> ('-')
            //       |
            //       +--- ('-') --> Decrement ('--')
            //       |
            //       +--- ('=') --> Assignment subtraction ('-=')
            //       |
            //       +--> Arithmetic subtraction ('-')
            addToken(match('-') ? TokenType::OP_DECREMENT : (match('=') ? TokenType::ASSIGNMENT_OP_SUB : TokenType::ARITHMETIC_OP_SUB));
            break;
        case '*':
            // --> ('*')
            //       |
            //       +--- ('=') --> Assignment multiplication ('*=')
            //       |
            //       +--> Arithmetic multiplication ('*')
            addToken(match('=') ? TokenType::ASSIGNMENT_OP_MUL : TokenType::ARITHMETIC_OP_MUL);
            break;
        case '/':
            // --> ('/')
            //       |
            //       +--- ('/') --> Single-line comment
            //       |
            //       +--- ('*') --> Multi-line comment
            //       |
            //       +--- ('=') --> Assignment division ('/=')
            //       |
            //       +--> Arithmetic division ('/')
            addToken(match('/') ? extractSingleLineComment() : (match('*') ? extractMultiLineComment() : (match('=') ? TokenType::ASSIGNMENT_OP_DIV : TokenType::ARITHMETIC_OP_DIV)));
            break;
        case '%':
            // --> ('%')
            //       |
            //       +--- ('=') --> Assignment modulus ('%=')
            //       |
            //       +--> Arithmetic modulus ('%')
            addToken(match('=') ? TokenType::ASSIGNMENT_OP_MOD : TokenType::ARITHMETIC_OP_MOD);
            break;
        case '&':
            // --> ('&')
            //       |
            //       +--- ('&') --> Boolean AND ('&&')
            //       |
            //       +--- ('=') --> Assignment AND ('&=')
            //       |
            //       +--> Bitwise AND ('&')
            addToken(match('&') ? TokenType::BOOLEAN_OP_AND : (match('=') ? TokenType::ASSIGNMENT_OP_AND : TokenType::BITWISE_OP_AND));
            break;
        case '^':
            // --> ('^')
            //       |
            //       +--- ('^') --> Boolean XOR ('^^')
            //       |
            //       +--- ('=') --> Assignment XOR ('^=')
            //       |
            //       +--> Bitwise XOR ('^')
            addToken(match('&') ? TokenType::BOOLEAN_OP_XOR : (match('=') ? TokenType::ASSIGNMENT_OP_XOR : TokenType::BITWISE_OP_XOR));
            break;
        case '|':
            // --> ('|')
            //       |
            //       +--- ('|') --> Boolean OR ('||')
            //       |
            //       +--- ('=') --> Assignment OR ('|=')
            //       |
            //       +--> Bitwise OR ('|')
            addToken(match('&') ? TokenType::BOOLEAN_OP_OR : (match('=') ? TokenType::ASSIGNMENT_OP_OR : TokenType::BITWISE_OP_OR));
            break;
        case '<':
            // --> ('<')
            //       |
            //       +--- ('<') --> Bitwise left shift ('<<')
            //       |
            //       +--- ('=') --> Relational less than or equal to ('<=')
            //       |
            //       +--> Relational less than ('<')
            addToken(match('<') ? TokenType::BITWISE_OP_LEFT_SHIFT : (match('=') ? TokenType::RELATIONAL_OP_LESS_THAN_EQUAL : TokenType::RELATIONAL_OP_LESS_THAN));
            break;
        case '>':
            // --> ('>')
            //       |
            //       +--- ('>') --> Bitwise right shift ('>>')
            //       |
            //       +--- ('=') --> Relational greater than or equal to ('>=')
            //       |
            //       +--> Relational greater than ('>')
            addToken(match('>') ? TokenType::BITWISE_OP_RIGHT_SHIFT : (match('=') ? TokenType::RELATIONAL_OP_GREATER_THAN_EQUAL : TokenType::RELATIONAL_OP_GREATER_THAN));
            break;
        case '!':
            // --> ('!')
            //       |
            //       +--- ('=') --> Relational inequality ('!=')
            //       |
            //       +--> Boolean NOT ('!')
            addToken(match('=') ? TokenType::RELATIONAL_OP_INEQUALITY : TokenType::BOOLEAN_OP_NOT);
            break;
        case '\'':
            addToken(extractCharacterLiteral());
            break;
        default:
            if (isWhitespace(c)) {
                addToken(extractWhitespace());
            } else if (c == '0' && current() != '.') {
                addToken(extractZeroPrefixNumericLiteral());
            } else if (isDigit(c)) {
                addToken(extractNumericLiteral());
            } else if (c == '_' || isLetter(c)) {
                addToken(extractIdentifier());
            } else {
                addToken(extractUnrecognizedSymbol());
            }
            break;
    }
}

const std::string& Tokenizer::contentOnLine(const uint32_t line) {
    return module->source->content.at(line);
}

bool Tokenizer::atEnd(void) {
    const uint32_t total_lines = module->source->content.size();
    
    const bool past_last_line = head.line > total_lines - 1;
    const bool on_last_line = head.line == total_lines - 1;
    const bool on_end_of_line = head.column >= contentOnLine(head.line).size() - 1;
    const bool on_end_of_content = on_last_line && on_end_of_line;

    return past_last_line || on_end_of_content;
}

char Tokenizer::current(void) {
    return contentOnLine(head.line).at(head.column);
}

char Tokenizer::next(void) {
    const char c = current();

    if (c == '\n') {
        head.line += 1;
        head.column = 0;
    } else {
        head.column += 1;
    }

    lexeme_buffer.push_back(c);
    return c;
}

char Tokenizer::peek(void) {
    if (atEnd()) {
        return '\0';
    }

    if (current() == '\n') {
        return contentOnLine(head.line).at(0);
    }

    return contentOnLine(head.line).at(head.column + 1);
}

bool Tokenizer::match(const char expect) {
    if (atEnd() || current() != expect) {
        return false;
    }

    next();

    return true;
}

std::string Tokenizer::consume(void) {
    std::string lexeme = lexeme_buffer;

    lexeme_buffer.clear();

    tail = head;

    return lexeme;
}

void Tokenizer::addToken(const TokenType type) {
    switch (type) {
        case TokenType::WHITESPACE:
        case TokenType::COMMENT:
        case TokenType::UNKNOWN:
            consume();
            return;
        default:; 
    }

    tokens->push_back(createToken(type));

    // Token& tk = tokens->back();
    // std::cout << "[" << tk.index + 1 << "] "
    //     << tk.line + 1 << ":" << tk.column + 1 << ", "
    //     << "type[" << int(tk.type) << "], " 
    //     << "lexeme`" << tk.lexeme << "`" << std::endl;
}

Token Tokenizer::createToken(const TokenType type) {
    Token token = {};

    token.line = tail.line;
    token.column = tail.column;
    token.index = tokens->size();
    token.lexeme = consume();
    token.type = type;

    return token;
}

TokenType Tokenizer::error(const std::string message) {
    const Token error = createToken(TokenType::UNKNOWN);

    // xc: error: message   |< header
    //  --> file:ln:col     |< info
    //    :                 |< divider
    // ln | content         |< line content
    //    : underline       |< footer

    const std::string header = "xc: \033[31merror\033[0m: " + message + '\n';
    const std::string line_column = std::to_string(error.line + 1) + ':' + std::to_string(error.column + 1);
    const std::string info = " --> " + module->source->filename + ':' + line_column + '\n';

    std::string preview;
    {
        const std::string line_number = std::to_string(error.line + 1);
        const std::string divider = std::string(line_number.size() + 2, ' ') + ':';

        const std::string line_content = " " + line_number + " | " + contentOnLine(error.line);
        const std::string underline = std::string(min_of(error.lexeme.size(), contentOnLine(error.line).size() - error.column), '^');
        const std::string footer = divider + std::string(error.column + 1, ' ') + underline;

        preview = divider + '\n' + line_content + footer;
    }

    std::cerr << header << info << preview << std::endl;

    has_error = true;
    return TokenType::UNKNOWN;
}

TokenType Tokenizer::error(const std::string message, const char symbol) {
    return error(message + ": `" + std::string(1, symbol) + "`");
}

TokenType Tokenizer::extractSingleLineComment(void) {
    while (!atEnd() && !match('\n')) {
        next();
    }

    return TokenType::COMMENT;
}

TokenType Tokenizer::extractMultiLineComment(void) {
    while (!atEnd() && !(match('*') && match('/'))) {
        next();
    }

    return TokenType::COMMENT;
}

TokenType Tokenizer::extractIdentifier(void) {
    while (!atEnd() && (current() == '_' || isLetterOrDigit(current()))) {
        next();
    }

    if (reserved_words.count(lexeme_buffer) > 0) {
        return reserved_words.at(lexeme_buffer);
    }

    return TokenType::IDENTIFIER;
}

TokenType Tokenizer::extractWhitespace(void) {
    while (!atEnd() && isWhitespace(current())) {
        next();
    }
    
    return TokenType::WHITESPACE;
}

TokenType Tokenizer::extractZeroPrefixNumericLiteral(void) {
    if (match('b')) {
        return extractBinaryLiteral();
    } else if (match('o')) {
        return extractOctalLiteral();
    } else if (match('x')) {
        return extractHexadecimalLiteral();
    } else if (isLetterOrDigit(current()) || current() == '_') {
        while (!atEnd() && !(isSymbol(current()) || isWhitespace(current()))) {
            next();
        }

        return error("`0` should be alone or pair with `b`, `o`, or `x`");
    }
    return TokenType::INTEGER_LITERAL;
}

TokenType Tokenizer::extractBinaryLiteral(void) {
    if (!isBinaryDigit(current())) {
        return error("incomplete or invalid binary literal");
    }

    while (!atEnd() && isBinaryDigit(current())) {
        next();
    }

    if (isLetterOrDigit(current()) || current() == '_') {
        const char invalid = current();
        while (!atEnd() && !(isSymbol(current()) || isWhitespace(current()))) {
            next();
        }

        return error("invalid binary digit", invalid);
    }

    return TokenType::INTEGER_LITERAL;
}

TokenType Tokenizer::extractOctalLiteral(void) {
    if (!isOctalDigit(current())) {
        return error("incomplete or invalid octal literal");
    }

    while (!atEnd() && isOctalDigit(current())) {
        next();
    }

    if (isLetterOrDigit(current()) || current() == '_') {
        const char invalid = current();
        while (!atEnd() && !(isSymbol(current()) || isWhitespace(current()))) {
            next();
        }

        return error("invalid octal digit", invalid);
    }

    return TokenType::INTEGER_LITERAL;
}

TokenType Tokenizer::extractHexadecimalLiteral(void) {
    if (!isHexadecimalDigit(current())) {
        return error("incomplete or invalid hexadecimal literal");
    }

    while (!atEnd() && isHexadecimalDigit(current())) {
        next();
    }

    if (isLetterOrDigit(current()) || current() == '_') {
        const char invalid = current();
        while (!atEnd() && !(isSymbol(current()) || isWhitespace(current()))) {
            next();
        }

        return error("invalid hexadecimal digit", invalid);
    }

    return TokenType::INTEGER_LITERAL;
}

TokenType Tokenizer::extractNumericLiteral(void) {
    while (isDigit(current())) {
        next();
    }

    if (isDigit(peek()) && match('.')) {
        while (isDigit(current())) {
            next();
        }

        return TokenType::FLOAT_LITERAL;
    }

    return TokenType::INTEGER_LITERAL;
}

TokenType Tokenizer::extractCharacterLiteral(void) {
    if (match('\\')) {
        return extractEscapeSequence();
    }

    const char literal = next();
    if (literal == '\'') {
        return error("character literal cannot be empty");
    }

    if (isWhitespace(literal) && literal != ' ') {
        while (!atEnd() && !match('\'')) {
            next();
        }

        return error("whitespace other than ` ` is not allow in a character literal; use an escape sequence instead");
    }

    if (!match('\'')) {
        while (!atEnd() && !match('\'')) {
            next();
        }

        return error(atEnd() ? "missing terminating `\'`" : "too many characters for a character literal");
    }

    return TokenType::CHARACTER_LITERAL;
}

TokenType Tokenizer::extractEscapeSequence(void) {
    const char c = next();
    switch (c) {
        case 'n':
        case 't':
        case 'b':
        case 'r':
        case 'a':
        case '\'':
        case '\"':
        case '\\':
        case 'f':
        case 'v':
            break;
        default:
            // TODO: refactor this 
            if (c == '0') {
                if (match('b')) {
                    if (!isBinaryDigit(current())) {
                        while (!atEnd() && !match('\'')) {
                            next();
                        }

                        return error("incomplete or invalid binary literal");
                    }

                    while (!atEnd() && isBinaryDigit(current())) {
                        next();
                    }

                    if (isLetterOrDigit(current()) || current() == '_') {
                        const char invalid = current();
                        while (!atEnd() && !match('\'')) {
                            next();
                        }

                        return error("invalid binary digit", invalid); 
                    }
                } else if (match('o')) {
                    if (!isOctalDigit(current())) {
                        while (!atEnd() && !match('\'')) {
                            next();
                        }

                        return error("incomplete or invalid octal literal");
                    }

                    while (!atEnd() && isOctalDigit(current())) {
                        next();
                    }

                    if (isLetterOrDigit(current()) || current() == '_') {
                        const char invalid = current();
                        while (!atEnd() && !match('\'')) {
                            next();
                        }

                        return error("invalid octal digit", invalid); 
                    }
                } else if (match('x')) {
                    if (!isHexadecimalDigit(current())) {
                        while (!atEnd() && !match('\'')) {
                            next();
                        }

                        return error("incomplete or invalid hexadecimal literal");
                    }

                    while (!atEnd() && isHexadecimalDigit(current())) {
                        next();
                    }

                    if (isLetterOrDigit(current()) || current() == '_') {
                        const char invalid = current();
                        while (!atEnd() && !match('\'')) {
                            next();
                        }

                        return error("invalid hexadecimal digit", invalid); 
                    }
                } else if (isLetter(current()) || current() == '_') {
                    while (!atEnd() && !match('\'')) {
                        next();
                    }

                    return error("`0` should be alone or pair with `b`, `o`, or `x`");
                }
            } else if (isDigit(c)) {
                if (extractNumericLiteral() == TokenType::FLOAT_LITERAL) {
                    while (!atEnd() && !match('\'')) {
                        next();
                    }

                    return error("escape sequence cannot be a floating point value");
                }
            }
            break;
    }

    if (!match('\'')) {
        const char invalid = current();
        while (!atEnd() && !match('\'')) {
            next();
        }

        return error("invalid escape sequence", invalid);
    }

    return TokenType::CHARACTER_LITERAL;
}

TokenType Tokenizer::extractUnrecognizedSymbol(void) {
    while (!atEnd() && !isRecognized(current())) {
        next();
    }

    return error("Unrecognized symbol", lexeme_buffer.at(0));
}

std::optional<std::unique_ptr<TokenStream>> Tokenizer::extractTokenStream(void) {
    return has_error ? none() : some(std::move(tokens));
}