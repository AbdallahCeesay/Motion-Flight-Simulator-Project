#include "mbed.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

using namespace std::chrono;

/* Implementation of Digital Position Feedback for the old actuators (30mm/s) */
PwmOut RPWM(PC_8); // Retract PWM
PwmOut LPWM(PC_9); // Extend PWM

Timer timer;                                        // Timer to measure elapsed time
float delta_t = 0;                                  // Time elapsed in seconds
const float dutyCycle = 1;                          // Full speed
const float avgActuatorSpeed = 30.6827057;          // Actuator speed in mm/s
const float maxStroke = 300.0f;                     // Maximum actuator stroke in mm (0 to 300 mm)
float currentPosition = 0.0f;                       // Current position of the actuator (distance traveled)
bool isExtending = false;                           // Track if the actuator is extending

int main() {
    while (true) {
        char c;
        std::cin >> c;

        if (c == 'e') { // Extend actuator
            if (timer.elapsed_time().count() == 0) { // Start timer only if not already running
                timer.start();
                std::cout << "Timer starts... Extending actuator." << std::endl;
            }

            LPWM.write(1); 
            RPWM.write(0); 
            isExtending = true; // Set direction to extending
        }

        if (c == 'q') { // Retract actuator
            if (timer.elapsed_time().count() == 0) { // Start timer only if not already running
                timer.start();
                std::cout << "Timer starts... Retracting actuator." << std::endl;
            }

            RPWM.write(1); 
            LPWM.write(0); 
            isExtending = false; // Set direction to retracting
        }

        if (c == 's') { // Stop actuator and calculate position
            delta_t = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f; // Convert to seconds
            std::cout << "Timer stopped!" << std::endl;
            printf("The time taken was %.3f seconds\n\n", delta_t);

            RPWM.write(0); // Stop actuator
            LPWM.write(0);

            timer.stop(); // Stop the timer

            float distanceTraveled = (avgActuatorSpeed * dutyCycle) * delta_t;

            if (isExtending) { // Actuator was extending
                currentPosition += distanceTraveled; // Increase position
                if (currentPosition > maxStroke) {
                    currentPosition = maxStroke; // Clamp to maximum stroke
                }
            } 
            else 
            { // Actuator was retracting
                currentPosition -= distanceTraveled; // Decrease position
                if (currentPosition < 0) {
                    currentPosition = 0; // Clamp to minimum stroke
                }
            }

            //std::cout << "Distance Traveled: " << distanceTraveled << " mm" << std::endl;
            std::cout << "Current Position: " << currentPosition << " mm" << std::endl;

            timer.reset(); // Reset the timer for the next operation
        }
    }
}