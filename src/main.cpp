#include <iostream>

#include "../include/app/Application.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Not enough arguments.\n";
        return -1;
    }

    Application application;
    return application.run(argv[1]);
}
