#include <windows.h>
#include <iostream>
#include "stdint.h"

#define maximum_range 16000

// method 1 - splicing
//uint8_t tx_data; // 1 byte - easy
//uint16_t tx_data2;   // 2 bytes;  send this over 2 times

// method 2 - scaling down to fit within 1 byte
//int16_t tx_data;
//constexpr unsigned int scaling_factor = (maximum_range) / (1<<8);
//int8_t tx_data_scaled = tx_data / scaling_factor;   // this fits within a single byte


void sendByteToCOM3(uint8_t value);


int main() {
    uint8_t dataToSend = 1;  // Example value
    sendByteToCOM3(dataToSend);

    std::cout << "(PC) DONE!" << std::endl;
    return 0;
}

void sendByteToCOM3(uint8_t value) {
    HANDLE hSerial;
    DWORD bytesWritten;

    // Open COM3
    hSerial = CreateFile(L"\\\\.\\COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening COM3" << std::endl;
        return;
    }

    // Configure Serial Port
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting COM3 state" << std::endl;
        CloseHandle(hSerial);
        return;
    }

    dcbSerialParams.BaudRate = CBR_9600;  // Set baud rate to 9600
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting COM3 parameters" << std::endl;
        CloseHandle(hSerial);
        return;
    }

    // Write data
    if (!WriteFile(hSerial, &value, sizeof(value), &bytesWritten, NULL)) {
        std::cerr << "Error writing to COM3" << std::endl;
    }
    else {
        std::cout << "Sent value: " << static_cast<int>(value) << " to COM3" << std::endl;
    }

    // Close COM3
    CloseHandle(hSerial);
}




// 0000 : 0 
// 0001 : 1 (1<<0)
// 0010 : 2 (1<<1)
// 0100 : 4 (1<<2)
// 1000 : 8 (1<<3)

// therefore, (1<<n) = 2**n
// (1>>m) = /2**m
