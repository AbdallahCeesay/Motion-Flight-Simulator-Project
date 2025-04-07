#include "mbed.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

using namespace std::chrono;
using namespace std;

/* Implementation of Digital Position Feedback for the new actuators (50mm/s) */
PwmOut RPWM(PB_8); // Retract PWM
PwmOut LPWM(PB_9); // Extend PWM

Timer timer;                                        // Timer to measure elapsed time
float delta_t = 0;                                  // Time elapsed in seconds
const float dutyCycle = 1;                          // Full speed
const float maxStroke = 300.0f;                     // Maximum actuator stroke in mm (0 to 300 mm)
float currentPosition = 0.0f;                       // Current position of the actuator (distance traveled)
bool isExtending = false;                           // Track if the actuator is extending



int main()
{
    while (true) {

        char c;

        cin >> c; 

        if(c == 'e')
        {
            if (timer.elapsed_time().count() == 0) {
                timer.start();
                cout << "Timer start...Extending Actuator" << endl;

                LPWM.write(1);
                RPWM.write(0);

                isExtending = true;
            }
        }

        if(c == 'q') {

            if (timer.elapsed_time().count() == 0) {
                timer.start();
                cout << "Timer start...Retracting Actuator" << endl;

                LPWM.write(0);
                RPWM.write(1);

                isExtending = false;
            }
        }

        if(c == 's') {
            delta_t = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f; // Convert to seconds

            cout << "Timer Stopped!" << endl;
            printf("The time taken was %.3f seconds\n\n", delta_t);

            LPWM.write(0);
            RPWM.write(0);
            
            timer.stop();
            timer.reset();
        }

    }
}

