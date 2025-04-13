#include "mbed.h"
#include <cstdio>
#include <iostream>
#include <string>

using namespace std;

DigitalOut led(LED1);
DigitalOut led2(LED2);

PwmOut act1_RPWM(PC_8); 
PwmOut act1_LPWM(PC_9);

PwmOut act2_RPWM(PE_5);
PwmOut act2_LPWM(PE_6);

PwmOut act3_RPWM(PB_8); 
PwmOut act3_LPWM(PB_9);

PwmOut act4_RPWM(PA_5);
PwmOut act4_LPWM(PA_6);

PwmOut act5_RPWM(PD_14);
PwmOut act5_LPWM(PD_15);

PwmOut act6_RPWM(PE_9);
PwmOut act6_LPWM(PE_11);

void stopActuators();
void retractActuators();
void extendActuator();

int main()
{
    while (true) {

        char c;
        std::cin >> c;
        
        std::cout << "C: " << c << endl;

        if (c == 'e') {

            extendActuator();
        }

        if (c == 'q') {

            retractActuators();
        }

        if (c == 's') {
            
            stopActuators();
        }

        // for debugging
        cout << "act1_RPWM: " << act1_RPWM.read() << endl;
        cout << "act1_LPWM: " << act1_LPWM.read() << endl << endl;

        cout << "act2_RPWM: " << act2_RPWM.read() << endl;
        cout << "act2_LPWM: " << act2_LPWM.read() << endl << endl;

    }
}

void extendActuator()
{
    act1_LPWM = 0.6; // extend actuator
    act1_RPWM = 0;

    act2_LPWM = 0.6;
    act2_RPWM = 0;

    act3_LPWM = 1;
    act3_RPWM = 0;

    act4_LPWM = 1;
    act4_RPWM = 0;

    act5_LPWM = 1;
    act5_RPWM = 0;

    act6_LPWM = 1;
    act6_RPWM = 0;
}

void retractActuators() 
{
    act1_RPWM = 0.6; // retract actuator
    act1_LPWM = 0;

    act2_RPWM = 0.6;
    act2_LPWM = 0;

    act3_RPWM = 1;
    act3_LPWM = 0;

    act4_RPWM = 1;
    act4_LPWM = 0;

    act5_RPWM = 1;
    act5_LPWM = 0;

    act6_RPWM = 1;
    act6_LPWM = 0;
}

void stopActuators()
{
    act1_RPWM = 0; 
    act1_LPWM = 0;

    act2_RPWM = 0;
    act2_LPWM = 0;

    act3_RPWM = 0;
    act3_LPWM = 0;

    act4_RPWM = 0;
    act4_LPWM = 0;

    act5_RPWM = 0;
    act5_LPWM = 0;

    act6_RPWM = 0;
    act6_LPWM = 0;
}