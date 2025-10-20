#include <iostream>
#include <string>

void io_show_fn(const std::string& fluid) {
    std::cout << fluid << std::endl;
}

std::string io_get_fn(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

int main(int argc, char** argv) {
    io_show_fn("hola Por favor, ingrese su nombre:");
    std::string result_temp_1 = io_get_fn("> ");
    std::cout << "Input received: " << result_temp_1 << std::endl;
    return 0;
}
