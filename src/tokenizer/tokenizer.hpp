#ifndef F_TOKENIZER_HPP
#define F_TOKENIZER_HPP

#include <string>
#include <vector>

enum class TokenType {
    TOKEN_EOF,           // End of File
    TOKEN_IDENTIFIER,    // f, io, z, show, assign, #HEAD, CALL, %CALL_LIBRARY
    TOKEN_OPERATOR,      // ++, +++, >>
    TOKEN_FLUID,         // Flujo de cadena
    TOKEN_LPAREN,        // (
    TOKEN_RPAREN,        // )
    TOKEN_LBRACE,        // {
    TOKEN_RBRACE,        // }
    TOKEN_LBRACKET,      // [
    TOKEN_RBRACKET,      // ]
    TOKEN_SEMICOLON,     // ;
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;
};

class Tokenizer {
public:
    Tokenizer(const std::string& sourceCode);
    std::vector<Token> tokenize();

private:
    const std::string& source;
    size_t position = 0;
    int currentLine = 1;
    int currentCol = 1;

    char peek();
    char consume();
    Token createToken(TokenType type, const std::string& value);
};

#endif 
