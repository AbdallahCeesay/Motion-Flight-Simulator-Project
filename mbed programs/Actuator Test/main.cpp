#include "mbed.h"
#include <cstdio>
#include <iostream>
#include <string>

DigitalOut led(LED1);
DigitalOut led2(LED2);

PwmOut RPWM(PC_8); 
PwmOut LPWM(PC_9);

int main()
{
    while (true) {

        char c;
        std::cin >> c;
        
        std::cout << "C: " << c << endl;

        if (c == 'e') {
            led = !led;
            LPWM.write(1.0f); // extend actuator
            RPWM.write(0);
        }

        if (c == 'q') {
            led2 = !led2;
            RPWM.write(1.0f); // retract actuator
            LPWM.write(0);
        }

        if (c == 's') {
            RPWM.write(0); 
            LPWM.write(0);
        }

        float rpwm_output = RPWM.read();
        float lpwm_output = LPWM.read();

        // for debugging
        std::cout << "rpwm_output: " << rpwm_output << endl;
        std::cout << "lpwm_output: " << lpwm_output << endl << endl;

        //wait_us(100000);
    }
}

