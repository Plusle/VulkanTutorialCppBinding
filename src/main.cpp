#include <application.hpp>
#include <iostream>

int main() {
    Application app;
    try {
        app.run();
    } catch(std::exception& e) {
        std::cout << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}