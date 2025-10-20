#include <iostream>
#include <string>

void io_show_fn(const std::string& fluid) {
    std::cout << fluid << std::endl;
}

int main(int argc, char** argv) {
    io_show_fn("hello world! how are you");

    return 0;
}