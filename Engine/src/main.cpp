#include <iostream>
#include <exception>

#include "application.h"

int main() {
    try {
        SnowEngine::Application app{};
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}