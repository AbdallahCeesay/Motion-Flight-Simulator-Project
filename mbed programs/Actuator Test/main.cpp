#include "mbed.h"
#include <cstdio>
#include <iostream>
#include <string>

DigitalOut led(LED1);
DigitalOut led2(LED2);

PwmOut act1_RPWM(PC_8); 
PwmOut act1_LPWM(PC_9);
PwmOut act2_RPWM(PE_5);
PwmOut act2_LPWM(PE_6);

int main()
{
    while (true) {

        char c;
        std::cin >> c;
        
        std::cout << "C: " << c << endl;

        if (c == 'e') {
            led = !led;
            act1_LPWM.write(1.0f); // extend actuator
            act1_RPWM.write(0);

            act2_LPWM.write(1.0f);
            act2_RPWM.write(0);
        }

        if (c == 'q') {
            led2 = !led2;
            act1_RPWM.write(1.0f); // retract actuator
            act1_LPWM.write(0);

            act2_RPWM.write(1.0f);
            act2_LPWM.write(0);
        }

        if (c == 's') {
            act1_RPWM.write(0); 
            act1_LPWM.write(0);

            act2_RPWM.write(0);
            act2_LPWM.write(0);
        }

        float act1_rpwm_output = act1_RPWM.read();
        float act1_lpwm_output = act1_LPWM.read();

        float act2_rpwm_output = act2_RPWM.read();
        float act2_lpwm_output = act2_LPWM.read();

        // for debugging
        std::cout << "act1_RPWM: " << act1_rpwm_output << endl;
        std::cout << "act1_LPWM: " << act1_lpwm_output << endl << endl;

        std::cout << "act2_RPWM: " << act2_rpwm_output << endl;
        std::cout << "act2_LPWM: " << act2_lpwm_output << endl << endl;

    }
}

