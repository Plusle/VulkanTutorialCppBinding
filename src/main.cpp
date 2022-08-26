#include <application.hpp>
#include <iostream>

int main() {
    Application app;
    try {
        app.run();
    } catch(...) {
        std::cout << "Error\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}