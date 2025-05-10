#ifndef DIGI_POS_FEEDBACK_HPP
#define DIGI_POS_FEEDBACK_HPP

#include "mbed.h"
#include <chrono>
#include <iostream> // Include for potential debugging if needed inside class
#include <iomanip>  // Include for potential debugging if needed inside class

using namespace std::chrono;

class DigitalPosFeedback {
private:
    float DUTY_CYCLE;
    float ACTUATOR_SPEED; // mm per second
    static constexpr float MAX_STROKE = 300.0f; // mm

    Timer timer;
    float lastTime; // seconds

    PwmOut RPWM;
    PwmOut LPWM;

public:
    float currentPosition; // Estimated position in mm

    enum class ActuatorState {
        EXTENDING,
        RETRACTING,
        STOPPED
    };

    ActuatorState state;

    // Constructor
    DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed = 30.6827057f, float duty = 0.6f); // Using provided default speed and duty

    // --- New Control Methods ---
    void extend();
    void retract();
    void stop();
    // -------------------------

    // Removed handleInput as control is now external

    void setDuty_Cycle(float duty); // Renamed for consistency
    void setActuatorSpeed(float speed);
    void updatePosition();
    void printPosition(const char* label);
    void resetPosition(); // Utility to reset estimated position
    float getEstimatedPosition(); // Getter for estimated position
};

#endif // DIGI_POS_FEEDBACK_HPP