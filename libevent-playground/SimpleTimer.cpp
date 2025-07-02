//
// Created by Tmax on 2025-07-02.
//

#include "SimpleTimer.h"

int main() {
    try {
        SimpleTimer timer{};
        timer.start();
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << std::endl;
        return -1;
    }
    return 0;
}