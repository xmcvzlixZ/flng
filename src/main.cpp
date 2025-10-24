#include "tokenizer.hpp"
#include "parser.hpp"
#include "file_reader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

mutex consoleMutex;

string readSourceFile(const string& path) {
    ifstream ifs(path);
    if (!ifs.is_open()) {
        lock_guard<mutex> lock(consoleMutex);
        cerr << "Error: No se pudo encontrar el archivo: " << path << endl;
        exit(1);
    }
    stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void generateOutputCppCode(const Parser& parser, const string& outputFileName) {
    ofstream ofs(outputFileName);
    if (!ofs.is_open()) {
        lock_guard<mutex> lock(consoleMutex);
        cerr << "Error: No se pudo crear el archivo de salida C++: " << outputFileName << endl;
        exit(1);
    }

    ofs << "#include <iostream>\n";
    ofs << "#include <string>\n\n";

    bool is_get_used = false;
    bool is_show_used = false;
    for (const auto& pair : parser.libraryAssignments) {
        if (pair.second == "get") is_get_used = true;
        else if (pair.second == "show") is_show_used = true;
    }

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

    ofs << "int main(int argc, char** argv) {\n";

    for (size_t i = 0; i < parser.apiCalls.size(); ++i) {
        const auto& call = parser.apiCalls[i];
        string nativeApiRole = parser.libraryAssignments.at(call.functionName);
        string cppFunctionName = call.moduleName + "_" + nativeApiRole + "_fn";
        string cleanFluidContent = call.fluidContent.substr(1, call.fluidContent.length() - 2);

        if (nativeApiRole == "get") {
            string resultVarName = "result_temp_" + to_string(i);
            ofs << "    std::string " << resultVarName << " = " << cppFunctionName << "(\"" << cleanFluidContent << "\");\n";
            ofs << "    std::cout << \"Input received: \" << " << resultVarName << " << std::endl;\n";
        } else {
            ofs << "    " << cppFunctionName << "(\"" << cleanFluidContent << "\");\n";
        }
    }

    ofs << "    return 0;\n";
    ofs << "}\n";

    lock_guard<mutex> lock(consoleMutex);
    if (parser.apiCalls.empty()) {
        cout << "Traduccion: Codigo C++ generado en '" << outputFileName << "' (programa vacio)." << endl;
    } else {
        cout << "Traduccion: Codigo C++ generado en '" << outputFileName << "'" << endl;
    }
}

void compileFile(const string& sourceFilePath) {
    string outputExecName;
    size_t dotPos = sourceFilePath.rfind('.');
    if (dotPos != string::npos) {
        outputExecName = sourceFilePath.substr(0, dotPos);
    } else {
        outputExecName = sourceFilePath;
    }
    
    string tempCppFileName = outputExecName + ".cpp";

    {
        lock_guard<mutex> lock(consoleMutex);
        cout << "[" << sourceFilePath << "] Iniciando proceso de traduccion..." << endl;
    }

    string sourceCode = readSourceFile(sourceFilePath);

    string basePath = ".";
    size_t lastSlash = sourceFilePath.rfind('/');
    if (lastSlash != string::npos) {
        basePath = sourceFilePath.substr(0, lastSlash);
    }

    Tokenizer tokenizer(sourceCode);
    vector<Token> tokens = tokenizer.tokenize();

    Parser parser(tokens, basePath);
    parser.parse();

    generateOutputCppCode(parser, tempCppFileName);

    {
        lock_guard<mutex> lock(consoleMutex);
        cout << "[" << sourceFilePath << "] Compilando codigo C++ intermedio..." << endl;
    }
    
    string compileCommand = "g++ -std=c++17 " + tempCppFileName + " -o " + outputExecName;
    
    if (system(compileCommand.c_str()) != 0) {
        lock_guard<mutex> lock(consoleMutex);
        cerr << "[" << sourceFilePath << "] Error: Fallo al compilar el codigo C++ intermedio." << endl;
        return;
    }

    {
        lock_guard<mutex> lock(consoleMutex);
        cout << "[" << sourceFilePath << "] FINALIZADO. El ejecutable esta listo: ./" << outputExecName << endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <archivo1.f> <archivo2.f> ..." << endl;
        return 1;
    }

    cout << "Compilador F V1.0 - Compilacion Multi-hilo" << endl;
    
    vector<thread> threads;
    for (int i = 1; i < argc; ++i) {
        threads.emplace_back(compileFile, string(argv[i]));
    }

    for (auto& th : threads) {
        th.join();
    }

    cout << "Todos los trabajos de compilacion han finalizado." << endl;
    
    return 0;
}
