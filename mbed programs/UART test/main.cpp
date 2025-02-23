#include "mbed.h"
#include "stdint.h"
#include <cstdint>
#include <iostream>

static UnbufferedSerial serial_port(USBTX,USBRX);

// method 1

// int8_t rx_data_lsb;     // lowest byte
// int8_t rx_data_msb;     // highest byte
// int16_t rx_data_final;  // final concatenated 2 bytes

// method 2
int8_t rx_data;

int main()
{

    // Set desired properties (9600-8-N-1).
    serial_port.baud(9600);
    serial_port.format(
        /* bits */ 8,
        /* parity */ SerialBase::None,
        /* stop bit */ 1
    );

    // wait for uart3 to receive something
    // RXNE flag
    while(!(USART3->SR & 0x20));

    // once received,read value
    // N.B. reading clears flag automatically
    rx_data = USART3->DR;

    // print out data
    std::cout << "Data received is: " << (unsigned int)rx_data << std::endl; 

}

