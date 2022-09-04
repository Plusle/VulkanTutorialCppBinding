#include <application.hpp>
#include <iostream>
#include <limits>

int main() {
    // std::cout << std::numeric_limits<uint32_t>::max() << "\n";


    Application app;
    try {
        app.run();
    } catch(std::exception& e) {
        std::cout << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}