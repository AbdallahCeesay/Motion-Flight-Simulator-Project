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
    float targetPosition = 0.0f;                            // New: target position to move toward
    float tolerance = 10.0f;                                 // New: acceptable error margin

    enum class ActuatorState {
        EXTENDING,
        RETRACTING,
        STOPPED
    };

    ActuatorState state = ActuatorState::STOPPED;

    DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed = 30.6827057f, float duty = 1.0f);

    void setDuty_Cycle(float duty);
    void setActuatorSpeed(float speed);
    void updateStateFromPWM();
    void updatePosition();
    void moveToTarget();       // <--- NEW function declaration
    void printPosition(const char* label);
    void extend();
    void retract();
    void stop();

    float getDutyCycle() const {
        return DUTY_CYCLE;
    }
};

#endif 
