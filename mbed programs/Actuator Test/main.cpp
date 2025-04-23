#include "mbed.h"
#include <cstdio>
#include <iostream>
#include <string>
#include "DigiPosFeedback_Lib/DigiPosFeedback.hpp"


using namespace std;

DigitalOut led(LED1);
DigitalOut led2(LED2);

static BufferedSerial terminal(USBTX, USBRX, 115200);

int main()
{
    terminal.set_blocking(false);                               // Non‑blocking reads on the terminal
    float ACTUATOR_SPEED = 30.6827057f;

    DigitalPosFeedback actuator1 (PC_8, PC_9, ACTUATOR_SPEED, 0.6);
    DigitalPosFeedback actuator2 (PE_5, PE_6, ACTUATOR_SPEED, 0.6);
    DigitalPosFeedback actuator3 (PB_8, PB_9);
    DigitalPosFeedback actuator4 (PA_5, PA_6);
    DigitalPosFeedback actuator5 (PD_14, PD_15);
    DigitalPosFeedback actuator6 (PE_9, PE_11);



    while (true) {

        // Clear the terminal screen and reset cursor to top
        std::cout << "\033[2J\033[H";                           
        // ANSI characters. \033[2J → Clear the entire screen & \033[H → Move the cursor to the top-left corner (home)

        char inputChar = '\0';

        if(terminal.readable()) {
            terminal.read(&inputChar, 1);
        }

        /*Actuator 1*/
        actuator1.handleInput(inputChar);
        actuator1.updatePosition();
        actuator1.printPosition("Actuator 1");

        /*Actuator 2*/
        actuator2.handleInput(inputChar);
        actuator2.updatePosition();
        actuator2.printPosition("Actuator 2");

        /*Actuator 3*/
        actuator3.handleInput(inputChar);
        actuator3.updatePosition();
        actuator3.printPosition("Actuator 3");

        /*Actuator 4*/
        actuator4.handleInput(inputChar);
        actuator4.updatePosition();
        actuator4.printPosition("Actuator 4");

        /*Actuator 5*/
        actuator5.handleInput(inputChar);
        actuator5.updatePosition();
        actuator5.printPosition("Actuator 5");

        /*Actuator 6*/
        actuator6.handleInput(inputChar);
        actuator6.updatePosition();
        actuator6.printPosition("Actuator 6");
        
        ThisThread::sleep_for(2ms);
    }
}
