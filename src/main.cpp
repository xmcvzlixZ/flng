#include "tokenizer.hpp"
#include "parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib> 
#include <stdexcept> 

using namespace std;

string readSourceFile(const string& path) {
    ifstream ifs(path);
    if (!ifs.is_open()) {
        cerr << "Error: No se pudo encontrar el archivo: " << path << endl;
        exit(1);
    }
    stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

string readFileToString(const string& path) {
    return readSourceFile(path);
}

// NUEVA FUNCIÓN: Generar código C++ secuencialmente para todas las llamadas o para un programa vacío.
void generateOutputCppCode(const Parser& parser, const string& outputFileName) {
    
    ofstream ofs(outputFileName);
    if (!ofs.is_open()) {
        cerr << "Error: No se pudo crear el archivo de salida C++: " << outputFileName << endl;
        exit(1);
    }

    // 1. Escribir encabezados C++
    ofs << "#include <iostream>\n";
    ofs << "#include <string>\n\n";

    // 2. Determinar qué APIs fueron usadas para generar solo esas definiciones.
    bool is_get_used = false;
    bool is_show_used = false;
    
    for (const auto& pair : parser.libraryAssignments) {
        if (pair.second == "get") {
            is_get_used = true;
        } else if (pair.second == "show") {
            is_show_used = true;
        }
    }

    // 3. Definiciones simuladas de APIs
    if (is_show_used) {
        ofs << "void io_show_fn(const std::string& fluid) {\n";
        ofs << "    std::cout << fluid << std::endl;\n";
        ofs << "}\n\n";
    }

    if (is_get_used) {
        ofs << "std::string io_get_fn(const std::string& prompt) {\n";
        ofs << "    std::string input;\n";
        ofs << "    std::cout << prompt;\n";
        ofs << "    std::getline(std::cin, input);\n";
        ofs << "    return input;\n";
        ofs << "}\n\n";
    }

    // 4. Escribir inicio de main()
    ofs << "int main(int argc, char** argv) {\n";

    // 5. Escribir llamadas a la API (ITERACIÓN CLAVE)
    for (size_t i = 0; i < parser.apiCalls.size(); ++i) {
        const auto& call = parser.apiCalls[i];
        
        // La validación de existencia de la función se hizo en el parser, aquí asumimos que existe.
        string nativeApiRole = parser.libraryAssignments.at(call.functionName);
        string cppFunctionName = call.moduleName + "_" + nativeApiRole + "_fn";
        string cleanFluidContent = call.fluidContent.substr(1, call.fluidContent.length() - 2);

        if (nativeApiRole == "get") {
            // Manejar 'get'. Se usa un nombre de variable único basado en el índice.
            string resultVarName = "result_temp_" + to_string(i);
            ofs << "    std::string " << resultVarName << " = " << cppFunctionName << "(\"" << cleanFluidContent << "\");\n";
            // La línea de depuración que vimos en tu ejemplo (imprime lo recibido inmediatamente)
            ofs << "    std::cout << \"Input received: \" << " << resultVarName << " << std::endl;\n"; 
        } else {
            // Manejar 'show' o cualquier otra API
            ofs << "    " << cppFunctionName << "(\"" << cleanFluidContent << "\");\n";
        }
    }

    // 6. Escribir fin de main()
    ofs << "    return 0;\n";
    ofs << "}\n";

    if (parser.apiCalls.empty()) {
        cout << "Traduccion: Codigo C++ generado en '" << outputFileName << "' (programa vacio)." << endl;
    } else {
        cout << "Traduccion: Codigo C++ generado en '" << outputFileName << "'" << endl;
    }
}

int main(int argc, char** argv) {
    if (argc != 5 || string(argv[1]) != "f" || string(argv[3]) != "p-") {
        cerr << "Uso: " << argv[0] << " f <archivo.f> p- <ejecutable_salida>" << endl;
        return 1;
    }

    string sourceFilePath = argv[2];
    string outputExecName = argv[4];
    string tempCppFileName = outputExecName + ".cpp";

    cout << "Compilador F V1.0 - Iniciando proceso de traduccion..." << endl;
    
    string sourceCode = readSourceFile(sourceFilePath);

    Tokenizer tokenizer(sourceCode);
    vector<Token> tokens = tokenizer.tokenize();

    Parser parser(tokens);
    parser.parse();
    
    // 4. Generación de Código C++ (Traducción real)
    generateOutputCppCode(parser, tempCppFileName);
    
    // 5. Compilación del código C++ generado
    cout << "Compilacion: Compilando codigo C++ intermedio..." << endl;
    string compileCommand = "g++ -std=c++17 " + tempCppFileName + " -o " + outputExecName;
    
    if (system(compileCommand.c_str()) != 0) {
        cerr << "Error: Fallo al compilar el codigo C++ intermedio." << endl;
        return 1;
    }

    cout << "FINALIZADO. El ejecutable de F esta listo: ./" << outputExecName << endl;
    
    return 0;
}
