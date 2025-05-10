#ifndef DIGITALPOSFEEDBACK_HPP
#define DIGITALPOSFEEDBACK_HPP

#include "mbed.h"
#include <chrono>
#include <cmath> // For fabsf

class DigitalPosFeedback {
private:
    float DUTY_CYCLE = 1.0f;            // Default duty cycle (can be overridden)
    float ACTUATOR_SPEED = 30.6827057f; // Default speed in mm/s (can be overridden)


    Timer timer;                        // For time tracking
    float lastTime_s;                   // Store time in seconds for calculations

    PwmOut RPWM;                        // PWM pin for retraction
    PwmOut LPWM;                        // PWM pin for extension

    // Internal state update based on commands
    void setState(ActuatorState newState);

public:
    // Made MAX_STROKE public const for access in main, ensure it's correct for your actuators
    static constexpr float MAX_STROKE = 300.0f; // Max actuator stroke in mm

    float currentPosition = 0.0f;       // Position estimate (STROKE) in mm
    float targetPosition = 0.0f;        // Target position (STROKE) to move toward
    float tolerance = 10.0f;            // Acceptable error margin (STROKE) in mm

    enum class ActuatorState {
        EXTENDING,
        RETRACTING,
        STOPPED
    };

    ActuatorState state = ActuatorState::STOPPED;

    // Constructor with optional speed and duty cycle
    DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed = 30.6827057f, float duty = 1.0f);

    // --- Configuration ---
    void setDuty_Cycle(float duty);
    void setActuatorSpeed(float speed);
    void setTolerance(float tol);

    // --- Control ---
    void updatePosition();     // Update estimated position based on state and time delta
    void moveToTarget();       // Decide whether to extend, retract, or stop based on target
    void extend();             // Command extension
    void retract();            // Command retraction
    void stop();               // Command stop

    // --- Status ---
    void printPosition(const char* label); // Print estimated position
    float getCurrentPosition() const;
    float getTargetPosition() const;
    ActuatorState getCurrentState() const;
    float getDutyCycle() const;

};

#endif // DIGITALPOSFEEDBACK_HPP