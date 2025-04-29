#include <Windows.h>
#include <tchar.h>
#include "SimConnect.h"
#include <iostream>

int main() {
    HANDLE hSimConnect = nullptr;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Flight Data Test", nullptr, 0, 0, 0))) {
        std::cout << "Connected to MSFS via SimConnect!" << std::endl;
        SimConnect_Close(hSimConnect);
    }
    else {
        std::cout << "Failed to connect to MSFS." << std::endl;
    }

    return 0;
}