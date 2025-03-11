#include "mbed.h"
#include <cstdio>
#include <iostream>

I2C i2c(PB_9, PB_8);

int main()
{
    char data[2]; 
    int BNO085_ADDR = 0x4A; 
    int numberOfBytesToRead = 2;
    i2c.frequency(1000000);

    while (true) {

        int i2c_flag = i2c.read(BNO085_ADDR, data, numberOfBytesToRead, 1);

        // Check if the read operation was successful
        if (i2c_flag == 0) {
            std::cout << "IMU Detected" << std::endl;

            for (int i = 0; i < numberOfBytesToRead; i++) {
                std::cout << "Data[" << i << "]: " << static_cast<int>(data[i]) << std::endl;
            }
        } else {
            std::cout << "IMU NOT Detected. Error: " << i2c_flag << std::endl;
        }

        wait_us(1000000); 
    }
}