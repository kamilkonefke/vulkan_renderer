#include <iostream>

#include "gfx.hpp"

int main() {
    std::cout << "Hello, vulkan!" << '\n';

    Gfx app;

    try {
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
