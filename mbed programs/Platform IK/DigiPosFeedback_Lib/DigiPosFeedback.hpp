#ifndef DIGITALPOSFEEDBACK_HPP
#define DIGITALPOSFEEDBACK_HPP

#include "mbed.h"
#include <chrono>

class DigitalPosFeedback {
private:
    float DUTY_CYCLE = 1.0f;                                // Full speed (default)
    float ACTUATOR_SPEED = 30.6827057f;                     // Speed in mm/s
    static constexpr float MAX_STROKE = 300.0f;             // Max actuator stroke in mm

    Timer timer;                                            // For time tracking
    float lastTime;

    PwmOut RPWM;                                      // PWM pin for retraction
    PwmOut LPWM;                                      // PWM pin for extension

public:
    float currentPosition = 0.0f;                           // Position estimate in mm

    enum class ActuatorState {
        EXTENDING,
        RETRACTING,
        STOPPED
    };

    ActuatorState state = ActuatorState::STOPPED;

    DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed = 30.6827057f, float duty = 1.0f);

    // Public methods
    void setDuty_Cycle(float duty);
    void setActuatorSpeed(float speed);
    void updateStateFromPWM();
    void updatePosition();
    void printPosition(const char* label);
};

#endif 
