/// *==============================================================*
///  cclass.hpp
///
///  c(haracter) class
///
///  Contains function use to classify characters similar to
///  the functions contained in "ctype.h". These functions are used
///  by the tokenizer.
/// *==============================================================*
#ifndef CCLASS_HPP
#define CCLASS_HPP

#include "common.hpp"

/// @return `true` if the character is a letter (A-Z, or a-z)
bool isLetter(const char c);

/// @return `true` if the character is a digit (0-9)
bool isDigit(const char c);

/// @return `true` if the character is either a letter or a digit
bool isLetterOrDigit(const char c);

/// @return `true` if the character is a hexadecimal digit (0-9, A-F, or a-f)
bool isHexadecimalDigit(const char c);

/// @return `true` if the character is a binary digit (0-1)
bool isBinaryDigit(const char c);

/// @return `true` if the character is an octal digit (0-7)
bool isOctalDigit(const char c);

/// @return `true` if the character is a whitespace character (e.g., space, tab)
bool isWhitespace(const char c);

/// @return `true` if the character is a symbol defined within the XC language specification
bool isSymbol(const char c);

/// @return `true` if the character is recognized by the XC language specification
bool isRecognized(const char c);

#endif /* CCLASS_HPP */