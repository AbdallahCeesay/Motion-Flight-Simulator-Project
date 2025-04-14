#include "mbed.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

using namespace std::chrono;

#define MAXIMUM_BUFFER_SIZE 32

// PWM outputs for actuator control
PwmOut RPWM(PC_8); // Retract PWM
PwmOut LPWM(PC_9); // Extend PWM

InterruptIn intRPWM(PC_8);
InterruptIn intLPWM(PC_9);

Timer timer;                                                 // Timer to measure elapsed time
static const float DUTY_CYCLE = 1.0f;                        // Full speed
static const float ACTUATOR_SPEED = 30.6827057f;             // Actuator speed in mm/s
static const float MAX_STROKE = 300.0f;                      // Maximum actuator stroke in mm
float currentPosition = 0.0f;                                // Current position of the stroke

// Actuator states using scoped enum for type safety
enum class ActuatorState {
    EXTENDING,
    RETRACTING,
    STOPPED
};


static BufferedSerial terminal(USBTX, USBRX);


int main() {

    // setting up the serial terminal
    terminal.set_baud(9600);
    terminal.set_format(8, BufferedSerial::None, 1);            // 8 bits, no parity, 1 stop bit
    terminal.set_blocking(false); 

    // application buffer to receive the data 
    char buffer[MAXIMUM_BUFFER_SIZE];

    // Start the timer and set last update time to the current timer value
    timer.start();
    float lastUpdatedTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;
    ActuatorState currentActuatorState = ActuatorState::STOPPED;  // Default state is stopped

    while (true) {

        if (terminal.read(&buffer, 1)) {
            char c;
            std::cin >> c;
            
            if (c == 'e') {
                currentActuatorState = ActuatorState::EXTENDING;
                LPWM = DUTY_CYCLE;
                RPWM = 0;
                std::cout << "Extending Actuator" << std::endl;
            }
            else if (c == 'q') {
                currentActuatorState = ActuatorState::RETRACTING;
                RPWM = DUTY_CYCLE;
                LPWM = 0;
                std::cout << "Retracting Actuator" << std::endl;
            }
            else if (c == 's') {
                currentActuatorState = ActuatorState::STOPPED;
                RPWM = 0;
                LPWM = 0;
                std::cout << "Stopped Actuator" << std::endl;
            }
        }
        
        // ----- Continuous Update of Position -----
        float currentTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;
        float delta_t = currentTime - lastUpdatedTime;
        
        // Only update if some time has passed
        if (delta_t > 0) {
            switch (currentActuatorState) {
                case ActuatorState::EXTENDING:
                    currentPosition += ACTUATOR_SPEED * DUTY_CYCLE * delta_t;
                    if (currentPosition > MAX_STROKE) {
                        currentPosition = MAX_STROKE;
                    }
                    break;
                    
                case ActuatorState::RETRACTING:
                    currentPosition -= ACTUATOR_SPEED * DUTY_CYCLE * delta_t;
                    if (currentPosition < 0.0f) {
                        currentPosition = 0.0f;
                    }                
                    break;
                    
                case ActuatorState::STOPPED:
                    // No movement: position remains the same
                    break;
            }
            
            // Update lastUpdatedTime for the next loop iteration
            lastUpdatedTime = currentTime;
        }
        
        // Print the current position continuously
        printf("Current Position: %.2f mm\n", currentPosition);
        
        // Sleep briefly to avoid spamming and allow other tasks to run
        ThisThread::sleep_for(10ms);
    }
}
