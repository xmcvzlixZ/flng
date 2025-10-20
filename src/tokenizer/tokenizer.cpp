#include "tokenizer.hpp"
#include <iostream>
#include <cctype> 
#include <vector>

using namespace std;

Tokenizer::Tokenizer(const std::string& sourceCode) : source(sourceCode) {}

char Tokenizer::peek() {
    if (position >= source.length()) return '\0';
    return source[position];
}

char Tokenizer::consume() {
    if (position >= source.length()) return '\0';
    char c = source[position++];
    if (c == '\n') {
        currentLine++;
        currentCol = 1;
    } else {
        currentCol++;
    }
    return c;
}

Token Tokenizer::createToken(TokenType type, const std::string& value) {
    return {type, value, currentLine, currentCol - (int)value.length()};
}

vector<Token> Tokenizer::tokenize() {
    vector<Token> tokens;
    string buffer;

    while (position < source.length()) {
        char c = peek();
        buffer = "";

        if (isspace(c)) {
            consume();
            continue;
        }

        if (c == '+' && source[position + 1] == '+') {
            consume(); consume();
            if (peek() == '+') { 
                consume();
                tokens.push_back(createToken(TokenType::TOKEN_OPERATOR, "+++"));
            } else { 
                tokens.push_back(createToken(TokenType::TOKEN_OPERATOR, "++"));
            }
            continue;
        }
        if (c == '>' && source[position + 1] == '>') {
            consume(); consume(); 
            tokens.push_back(createToken(TokenType::TOKEN_OPERATOR, ">>"));
            continue;
        }
        
        if (c == ';') { tokens.push_back(createToken(TokenType::TOKEN_SEMICOLON, ";")); consume(); continue; }
        if (c == '(') { tokens.push_back(createToken(TokenType::TOKEN_LPAREN, "(")); consume(); continue; }
        if (c == ')') { tokens.push_back(createToken(TokenType::TOKEN_RPAREN, ")")); consume(); continue; }
        if (c == '{') { tokens.push_back(createToken(TokenType::TOKEN_LBRACE, "{")); consume(); continue; }
        if (c == '}') { tokens.push_back(createToken(TokenType::TOKEN_RBRACE, "}")); consume(); continue; }
        if (c == '[') { tokens.push_back(createToken(TokenType::TOKEN_LBRACKET, "[")); consume(); continue; }
        if (c == ']') { tokens.push_back(createToken(TokenType::TOKEN_RBRACKET, "]")); consume(); continue; }

        if (c == '\'') {
            consume(); 
            buffer += c; 
            
            while (peek() != '\0' && peek() != '\'') {
                buffer += consume();
            }
            
            if (peek() == '\'') {
                buffer += consume(); 
                tokens.push_back(createToken(TokenType::TOKEN_FLUID, buffer));
                continue;
            } else {
                cerr << "Error Lexico: Flujo de cadena sin cerrar en linea " << currentLine << "." << endl;
                exit(1);
            }
        }
        
        if (isalnum(c) || c == '_' || c == '.' || c == '#' || c == '%') { 
            while (isalnum(peek()) || peek() == '_' || peek() == '.' || peek() == '#' || peek() == '%') {
                buffer += consume();
            }
            tokens.push_back(createToken(TokenType::TOKEN_IDENTIFIER, buffer)); 
            continue;
        }

        cerr << "Error Lexico: Caracter desconocido '" << c << "' en linea " << currentLine << "." << endl;
        exit(1);
    }
    
    tokens.push_back(createToken(TokenType::TOKEN_EOF, ""));
    return tokens;
}
