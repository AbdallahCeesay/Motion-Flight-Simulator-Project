#include "DigiPosFeedback.hpp"
#include <iostream>
#include <iomanip>
#include <chrono> // Make sure chrono is included

using namespace std::chrono;

// Constructor
DigitalPosFeedback::DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed, float duty)
    : RPWM(rpwm), LPWM(lpwm), ACTUATOR_SPEED(actuatorspeed), DUTY_CYCLE(duty), state(ActuatorState::STOPPED)
{
    // Initialize PWM periods if necessary (e.g., 20kHz)
    // RPWM.period_ms(20); // Example period, adjust if needed
    // LPWM.period_ms(20); // Example period, adjust if needed

    // Stop motors initially
    RPWM.write(0.0f);
    LPWM.write(0.0f);

    timer.start();
    // Initialize lastTime correctly using chrono
    lastTime_s = duration_cast<microseconds>(timer.elapsed_time()).count() / 1000000.0f;

    // Ensure initial position is reasonable (though it's set in main.cpp now)
    if (currentPosition < 0.0f) currentPosition = 0.0f;
    if (currentPosition > MAX_STROKE) currentPosition = MAX_STROKE;
}

// Set internal state and update PWM outputs accordingly
void DigitalPosFeedback::setState(ActuatorState newState) {
    if (state == newState) return; // No change

    state = newState;

    switch (state) {
        case ActuatorState::EXTENDING:
            LPWM.write(DUTY_CYCLE);
            RPWM.write(0.0f);
            break;
        case ActuatorState::RETRACTING:
            RPWM.write(DUTY_CYCLE);
            LPWM.write(0.0f);
            break;
        case ActuatorState::STOPPED:
        default:
            RPWM.write(0.0f);
            LPWM.write(0.0f);
            break;
    }
}


void DigitalPosFeedback::setDuty_Cycle(float duty) {
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;
    DUTY_CYCLE = duty;
    // Re-apply PWM if motor is currently moving
    setState(state);
}

void DigitalPosFeedback::setActuatorSpeed(float speed) {
    if (speed < 0.0f) speed = 0.0f;
    ACTUATOR_SPEED = speed;
}

void DigitalPosFeedback::setTolerance(float tol) {
    if (tol < 0.0f) tol = 0.0f;
    tolerance = tol;
}

void DigitalPosFeedback::printPosition(const char* label) {
    // Use printf for compatibility with potential retargeting in Mbed OS
    printf("%s Position: %.2f mm (State: %d)\n",
           label,
           currentPosition,
           static_cast<int>(state));
}


// This function ONLY updates the internal position estimate
void DigitalPosFeedback::updatePosition() {
    // Get current time in seconds
    float now_s = duration_cast<microseconds>(timer.elapsed_time()).count() / 1000000.0f;
    float dt = now_s - lastTime_s;

    if (dt > 0) { // Ensure time has passed
        switch (state) { // Calculate position change based on the CURRENT state
            case ActuatorState::EXTENDING:
                currentPosition += ACTUATOR_SPEED * dt;
                // Clamp to max stroke
                if (currentPosition > MAX_STROKE) {
                    currentPosition = MAX_STROKE;
                    // stop(); // Optional: stop if we hit the estimated limit
                }
                break;

            case ActuatorState::RETRACTING:
                currentPosition -= ACTUATOR_SPEED * dt;
                // Clamp to min stroke
                if (currentPosition < 0.0f) {
                    currentPosition = 0.0f;
                    // stop(); // Optional: stop if we hit the estimated limit
                }
                break;

            case ActuatorState::STOPPED:
                // No position change when stopped
                break;
        }
    }
    // Update lastTime for the next iteration
    lastTime_s = now_s;
}

// This function decides the NEXT state based on the error
void DigitalPosFeedback::moveToTarget() {
    // Error is difference between target stroke and current estimated stroke
    float error = targetPosition - currentPosition;

    // Check if we are within tolerance
    if (fabsf(error) <= tolerance) {
        stop(); // We are close enough, stop moving
    } else if (error > 0) {
        // Target is further extended than current position
        extend(); // Command extension
    } else {
        // Target is further retracted than current position (error < 0)
        retract(); // Command retraction
    }
}

// --- Explicit command functions ---

void DigitalPosFeedback::extend() {
    setState(ActuatorState::EXTENDING);
}

void DigitalPosFeedback::retract() {
    setState(ActuatorState::RETRACTING);
}

void DigitalPosFeedback::stop() {
    setState(ActuatorState::STOPPED);
}

// --- Status Getters ---
float DigitalPosFeedback::getCurrentPosition() const {
    return currentPosition;
}

float DigitalPosFeedback::getTargetPosition() const {
    return targetPosition;
}

DigitalPosFeedback::ActuatorState DigitalPosFeedback::getCurrentState() const {
    return state;
}

float DigitalPosFeedback::getDutyCycle() const {
    return DUTY_CYCLE;
}