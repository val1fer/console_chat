#pragma once

#include <iostream>
#include <string>
#include <cstdint>

void check_port(const char* port) {
    int check_port = std::stoi(port);
    if (check_port > UINT16_MAX) 
        std::cerr << "Your port is actually " << check_port%65535 << ". Range of valid ports: 0-65535\n";
}