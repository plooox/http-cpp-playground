//
// Created by Tmax on 2025-07-02.
//

#include "SimpleTCPServer.h"

int main() {
    try {
        SimpleTCPServer server(8080);
        server.start();
    } catch (const std::exception& ex) {
        std::cout << "[ERROR] " << ex.what() << std::endl;
    }
}