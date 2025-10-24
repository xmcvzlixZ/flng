#include "parser.hpp"
#include "file_reader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm> 

using namespace std;

Parser::Parser(const std::vector<Token>& tokens, const std::string& basePath) : tokens(tokens), basePath(basePath) {}

Token Parser::consume() {
    if (position >= tokens.size()) {
        return {TokenType::TOKEN_EOF, "", 0, 0}; 
    }
    return tokens[position++];
}

Token Parser::peek() {
    if (position >= tokens.size()) {
        return {TokenType::TOKEN_EOF, "", 0, 0};
    }
    return tokens[position];
}

void Parser::parseLibrary(const string& path, LibraryMap& map) {
    string fullPath = basePath + "/" + path;
    string source = readSourceFile(fullPath);
    Tokenizer tokenizer(source);
    vector<Token> libTokens = tokenizer.tokenize();
    
    size_t i = 0;
    string nativeApiName;
    
    while (i < libTokens.size()) {
        if (libTokens[i].value == "CALL" && i + 1 < libTokens.size() && libTokens[i+1].type == TokenType::TOKEN_FLUID) {
            string apiTokenValue = libTokens[i+1].value;
            nativeApiName = apiTokenValue.substr(1, apiTokenValue.length() - 2); 
            i += 2;
            break;
        }
        i++;
    }

    if (nativeApiName.empty()) {
        cerr << "Error Sintactico: La cabecera " << path << " no tiene declaracion 'CALL' valida." << endl;
        exit(1);
    }
    
    if (libTokens[i].value == "#HEAD") {
        i++;
        if (i < libTokens.size() && libTokens[i].type == TokenType::TOKEN_FLUID) {
            i++;
        }
    }

    if (libTokens[i].value == "f" && i + 1 < libTokens.size() && libTokens[i+1].type == TokenType::TOKEN_LPAREN) 
    {
        i += 2;
        
        bool hasApiArg = (libTokens[i].value == nativeApiName);
        if (hasApiArg) {
            i++; 
        }
        
        if (i + 1 < libTokens.size() && 
            libTokens[i].type == TokenType::TOKEN_RPAREN &&
            libTokens[i+1].type == TokenType::TOKEN_LBRACE) 
        {
            i += 2; 
            
            if (i + 9 < libTokens.size() &&
                libTokens[i].value == "assign" && 
                libTokens[i+1].value == "+++" && 
                libTokens[i+2].type == TokenType::TOKEN_FLUID && 
                libTokens[i+3].value == ">>" && 
                libTokens[i+4].type == TokenType::TOKEN_LPAREN && 
                libTokens[i+5].type == TokenType::TOKEN_LBRACKET && 
                libTokens[i+6].type == TokenType::TOKEN_RBRACKET && 
                libTokens[i+7].type == TokenType::TOKEN_RPAREN && 
                libTokens[i+8].type == TokenType::TOKEN_SEMICOLON && 
                libTokens[i+9].type == TokenType::TOKEN_RBRACE) 
            {
                string funcNameToken = libTokens[i+2].value;
                string funcName = funcNameToken.substr(1, funcNameToken.length() - 2); 
                
                map[funcName] = nativeApiName; 
                return; 
            }
        }
    }

    cerr << "Error Sintactico: La cabecera " << path << " tiene sintaxis de asignacion 'f(API) { assign... }' invalida." << endl;
    exit(1);
}

void Parser::parseEntryPoint(LibraryMap& map) {

    while (peek().value == "%CALL_LIBRARY") {
        consume(); 
        if (peek().type == TokenType::TOKEN_FLUID) {
            string path = peek().value;
            path = path.substr(1, path.length() - 2); 
            consume(); 
            
            parseLibrary(path, libraryAssignments); 
        }
    }
    
    if (peek().value != "f") {
        cerr << "Error Sintactico: El programa debe empezar con 'f'." << endl;
        exit(1);
    }
    consume(); 
    
    if (peek().type == TokenType::TOKEN_LPAREN) {
        consume();
        
        if (peek().type != TokenType::TOKEN_RPAREN) { 
            cerr << "Error Sintactico: El punto de entrada 'f()' no debe tener argumentos." << endl;
            exit(1);
        }
        consume(); 

    } else {
        cerr << "Error Sintactico: Se esperaba '()' despues de 'f' en el punto de entrada." << endl;
        exit(1);
    }

    if (peek().type != TokenType::TOKEN_LBRACE) {
        cerr << "Error Sintactico: Se esperaba '{' despues de la firma de 'f'." << endl;
        exit(1);
    }
    consume(); 
  
    parseApiCall(libraryAssignments);

    if (peek().type != TokenType::TOKEN_RBRACE) {
        cerr << "Error Sintactico: Se esperaba '}' al final del programa." << endl;
        exit(1);
    }
    consume();

    if (peek().type != TokenType::TOKEN_SEMICOLON) {
        cerr << "Error Sintactico: Se esperaba ';' al final de la definicion de 'f'." << endl;
        exit(1);
    }
    consume(); 
}

void Parser::parseApiCall(LibraryMap& map) {
    while (peek().type == TokenType::TOKEN_IDENTIFIER) {
        APICall currentCall;

        currentCall.moduleName = consume().value; 

        if (consume().value != "++") {
            cerr << "Error Sintactico: Se esperaba '++' despues del modulo '" << currentCall.moduleName << "'." << endl;
            exit(1);
        }

        currentCall.functionName = consume().value; 
        
        if (map.find(currentCall.functionName) == map.end()) {
            cerr << "Error Sintactico: Funcion/Rol '" << currentCall.functionName << "' no asignada en cabecera." << endl;
            exit(1);
        }

        if (peek().type != TokenType::TOKEN_LBRACKET) {
            cerr << "Error Sintactico: Se esperaba '[' para iniciar el flujo de datos para '" << currentCall.functionName << "'." << endl;
            exit(1);
        }
        consume(); 

        if (peek().type != TokenType::TOKEN_FLUID) {
            if (peek().type == TokenType::TOKEN_RBRACKET) {
                currentCall.fluidContent = "''"; 
            } else {
                cerr << "Error Sintactico: Se esperaba un flujo de cadena ('...') dentro de los corchetes." << endl;
                exit(1);
            }
        } else {
            currentCall.fluidContent = consume().value; 
        }
        
        if (peek().type != TokenType::TOKEN_RBRACKET) {
            cerr << "Error Sintactico: Se esperaba ']' para cerrar el flujo de datos." << endl;
            exit(1);
        }
        consume(); 

        if (consume().value != ";") {
            cerr << "Error Sintactico: Se esperaba ';' al final de la sentencia." << endl;
            exit(1);
        }
        
        apiCalls.push_back(currentCall); 
    }
}


void Parser::parse() {
    parseEntryPoint(libraryAssignments);
    if (peek().type != TokenType::TOKEN_EOF) {
        cerr << "Error Sintactico: Codigo basura despues de la funcion principal." << endl;
        exit(1);
    }
    cout << "Compilacion: Sintaxis de F validada con exito." << endl; 
}
