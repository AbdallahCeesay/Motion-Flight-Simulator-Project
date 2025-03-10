#include "mbed.h"
#include <cstdio>
#include <iostream>

I2C i2c(PB_9, PB_8);

int main()
{
    char data[2]; // Buffer to store the read data (2 bytes in this example)
    int BNO085_ADDR = 0x4A; // 7-bit I2C address
    int numberOfByteToRead = 2; // Number of bytes to read

    while (true) {
        // Perform I2C read operation
        int i2c_flag = i2c.read(BNO085_ADDR, data, numberOfByteToRead, 1);

        // Check if the read operation was successful
        if (i2c_flag == 0) {
            std::cout << "IMU Detected" << std::endl;

            // Print the data stored in the buffer
            for (int i = 0; i < numberOfByteToRead; i++) {
                std::cout << "Data[" << i << "]: " << static_cast<int>(data[i]) << std::endl;
            }
        } else {
            std::cout << "IMU NOT Detected. Error: " << i2c_flag << std::endl;
        }

        wait_us(1000000); // Wait for 1 second
    }
}