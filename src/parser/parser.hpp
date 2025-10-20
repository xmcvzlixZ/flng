#ifndef F_PARSER_HPP
#define F_PARSER_HPP

#include "tokenizer.hpp" 
#include <map>
#include <vector>
#include <string>

struct APICall {
    std::string moduleName;    
    std::string functionName;  
    std::string fluidContent;   
};

class Parser {
public: 
    using LibraryMap = std::map<std::string, std::string>;

    Parser(const std::vector<Token>& tokens);
    void parse();
    
    std::vector<APICall> apiCalls; 
    LibraryMap libraryAssignments; 

private:
    const std::vector<Token>& tokens;
    size_t position = 0;

    Token consume();
    Token peek();
    
    void parseLibrary(const std::string& path, LibraryMap& map); 
    void parseEntryPoint(LibraryMap& map);
    void parseApiCall(LibraryMap& map);
};

#endif 
