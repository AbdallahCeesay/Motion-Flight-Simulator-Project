#include "mbed.h"
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace std::chrono;

// PWM outputs for actuator control
PwmOut RPWM(PC_8);                                          // Retract PWM
PwmOut LPWM(PC_9);                                          // Extend PWM

Timer timer;                                                // Timer to measure elapsed time

static constexpr float DUTY_CYCLE     = 1.0f;               // Full speed
static constexpr float ACTUATOR_SPEED = 30.6827057f;        // Actuator speed in mm/s
static constexpr float MAX_STROKE     = 300.0f;             // Maximum actuator stroke in mm

float currentPosition = 0.0f;                               // Current position of the stroke

// Actuator states using scoped enum for type safety
enum class ActuatorState {
    EXTENDING,
    RETRACTING,
    STOPPED
};

static BufferedSerial terminal(USBTX, USBRX, 9600);

int main() {
    
    terminal.set_blocking(false);                               // Non‑blocking reads on the terminal

    // Start timer and initialize last update timestamp
    timer.start();
    float lastTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;

    ActuatorState state = ActuatorState::STOPPED;               // default state of the actuator is STOPPED (not moving)

    while (true) {
        /* 1) Check for incoming character */
        if (terminal.readable()) {
            char c;
            if (terminal.read(&c, 1) == 1) {
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
                        
                        break;                                  // ignore other characters
                }
            }
        }

        /* 2) Continuous update of position */
        float now = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;
        float dt  = now - lastTime;
        if (dt > 0) {
            switch (state) {
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

        std::cout << "Position: " << std::fixed << std::setprecision(2) << currentPosition << " mm\n";
        ThisThread::sleep_for(10ms);
    }
}
