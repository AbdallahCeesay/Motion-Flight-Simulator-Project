#include "mbed.h"
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace std::chrono;
static BufferedSerial terminal(USBTX, USBRX, 9600);

class DigitalPosFeedback {
    private:
        float DUTY_CYCLE = 1.0f;                                              // Full speed
        float ACTUATOR_SPEED = 30.6827057f;                                   // Actuator speed in mm/s
        static constexpr float MAX_STROKE = 300.0f;                           // Maximum actuator stroke in mm

        Timer timer;                                                          // Timer to measure elapsed time
        float lastTime;
        PwmOut RPWM;                                                          // retract actuator
        PwmOut LPWM;                                                          // extend actuator

    public:
        float currentPosition = 0.0f;                                         // Current position of the stroke
        
        enum class ActuatorState {                                            // Actuator states using scoped enum for type safety
            EXTENDING,
            RETRACTING,
            STOPPED 
        };

        ActuatorState state = ActuatorState::STOPPED;                           // default state of the actuator is STOPPED (not moving)

        DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed = 30.6827057f, float duty = 1);
    
        // member functions
        void setDuty_Cycle(float speed);
        void setActuatorSpeed(float speed);
        void handleInput(char c);
        void updatePosition();
        void printPosition(const char*);

};

// constructor to Initialise pins
DigitalPosFeedback::DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed, float duty) : RPWM(rpwm), LPWM(lpwm), ACTUATOR_SPEED(actuatorspeed), DUTY_CYCLE(duty)
{
    timer.start();
    lastTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;
}

// function definitions
void DigitalPosFeedback::setDuty_Cycle(float duty) {

    DUTY_CYCLE = duty;
}

void DigitalPosFeedback::setActuatorSpeed(float speed) {
    
    ACTUATOR_SPEED = speed;
}

void DigitalPosFeedback::printPosition(const char* label) {

    std::cout << label << " Position: " << std::fixed << std::setprecision(2) << currentPosition << " mm\n";
}

void DigitalPosFeedback::handleInput(char c) {
        
    switch (c) {
        case 'e':
            state = ActuatorState::EXTENDING;
            LPWM.write(DUTY_CYCLE);
            RPWM.write(0.0f);
            break;

        case 'q':
            state = ActuatorState::RETRACTING;
            RPWM.write(DUTY_CYCLE);
            LPWM.write(0.0f);
            break;

        case 's':
            state = ActuatorState::STOPPED;
            LPWM.write(0.0f);
            RPWM.write(0.0f);
            break;

        default:
            break;
    }
}

void DigitalPosFeedback::updatePosition() {

    float now = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;
    float dt  = now - lastTime;
    if (dt > 0) {
        switch (state)
        {
            case ActuatorState::EXTENDING:
                currentPosition += ACTUATOR_SPEED * dt;
                if (currentPosition > MAX_STROKE) {
                    currentPosition = MAX_STROKE;
                }
                break;
            case ActuatorState::RETRACTING:
                currentPosition -= ACTUATOR_SPEED * dt;
                if (currentPosition < 0.0f) {
                    currentPosition = 0.0f;
                }
                break;
            case ActuatorState::STOPPED:
                
                break;
        }
        lastTime = now;
    }
}

int main() {

    terminal.set_blocking(false);                               // Non‑blocking reads on the terminal
    float ACTUATOR_SPEED = 30.6827057f;   

    DigitalPosFeedback actuator1(PC_8, PC_9);
    DigitalPosFeedback actuator2(PB_8, PB_9,ACTUATOR_SPEED ,0.6);
    DigitalPosFeedback actuator3(PE_5, PE_6);

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

        ThisThread::sleep_for(2ms);
    }
}


