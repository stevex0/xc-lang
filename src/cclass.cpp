/// *==============================================================*
///  cclass.cpp
/// *==============================================================*
#include "include/cclass.hpp"

bool isLetter(const char c) {
    switch(c) {
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
            return true;
        default:
            return false;
    }
}
bool isDigit(const char c) {
    switch(c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return true;
        default:
            return false;
    }
}

bool isLetterOrDigit(const char c) {
    return isLetter(c) || isDigit(c);
}

bool isBinaryDigit(const char c) {
    switch(c) {
        case '0':
        case '1':
            return true;
        default:
            return false;
    }
}

bool isHexadecimalDigit(const char c) {
    switch(c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            return true;
        default:
            return false;
    }
}

bool isOctalDigit(const char c) {
    switch(c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            return true;
        default:
            return false;
    }
}

bool isWhitespace(const char c) {
    switch(c) {
        case ' ':
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
            return true;
        default:
            return false;
    }
}

bool isSymbol(const char c) {
    switch(c) {
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case ';':
        case ':':
        case ',':
        case '.':
        case '=':
        case '~':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '&':
        case '^':
        case '|':
        case '<':
        case '>':
        case '!':
        case '\'':
            return true;
        default:
            return false;
    }
}

bool isRecognized(const char c) {
    return isLetterOrDigit(c) || isWhitespace(c) || isSymbol(c) || c == '_';
}