#include "DigiPosFeedback.hpp"

// Constructor
DigitalPosFeedback::DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed, float duty)
    : RPWM(rpwm), LPWM(lpwm), ACTUATOR_SPEED(actuatorspeed), DUTY_CYCLE(duty), currentPosition(0.0f), state(ActuatorState::STOPPED)
{
    // Initialize PWM period if necessary (default is often 20ms)
    // RPWM.period_ms(20);
    // LPWM.period_ms(20);
    stop(); // Ensure actuators are stopped initially
    timer.start();
    lastTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;
}

void DigitalPosFeedback::setDuty_Cycle(float duty) {
    // Clamp duty cycle between 0.0 and 1.0
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;
    DUTY_CYCLE = duty;
    // Re-apply PWM if actuator is currently moving
    if (state == ActuatorState::EXTENDING) {
        LPWM.write(DUTY_CYCLE);
        RPWM.write(0.0f);
    } else if (state == ActuatorState::RETRACTING) {
        RPWM.write(DUTY_CYCLE);
        LPWM.write(0.0f);
    }
}

void DigitalPosFeedback::setActuatorSpeed(float speed) {
    ACTUATOR_SPEED = speed;
}

void DigitalPosFeedback::printPosition(const char* label) {
    // Using std::printf for potentially better performance on embedded
    printf("%s Position: %6.2f mm\n", label, currentPosition);
}

// --- New Control Methods Implementation ---
void DigitalPosFeedback::extend() {
    if (state != ActuatorState::EXTENDING) {
         // Only update state and timer if changing state
         lastTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f; // Update time before changing state
         state = ActuatorState::EXTENDING;
         LPWM.write(DUTY_CYCLE);
         RPWM.write(0.0f);
    }
}

void DigitalPosFeedback::retract() {
    if (state != ActuatorState::RETRACTING) {
         // Only update state and timer if changing state
         lastTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f; // Update time before changing state
         state = ActuatorState::RETRACTING;
         RPWM.write(DUTY_CYCLE);
         LPWM.write(0.0f);
    }
}

void DigitalPosFeedback::stop() {
     if (state != ActuatorState::STOPPED) {
         // Only update state if changing state
         lastTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f; // Update time before changing state
         state = ActuatorState::STOPPED;
         LPWM.write(0.0f);
         RPWM.write(0.0f);
     }
}
// ---------------------------------------

void DigitalPosFeedback::updatePosition() {
    float now = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;
    float dt = now - lastTime;

    // Prevent large jumps if updatePosition hasn't been called for a while
    // Especially important when changing states after a pause.
    // Limit dt to a reasonable maximum, e.g., 100ms (0.1s)
    // if (dt > 0.1f) dt = 0.1f; // Optional: depends on how often updatePosition is called

    if (dt > 0) { // Ensure time has actually passed
        switch (state) {
            case ActuatorState::EXTENDING:
                currentPosition += ACTUATOR_SPEED * dt;
                if (currentPosition > MAX_STROKE) {
                    currentPosition = MAX_STROKE;
                    // Optional: stop() automatically when limit reached
                    // stop();
                }
                break;

            case ActuatorState::RETRACTING:
                currentPosition -= ACTUATOR_SPEED * dt;
                if (currentPosition < 0.0f) {
                    currentPosition = 0.0f;
                     // Optional: stop() automatically when limit reached
                    // stop();
                }
                break;

            case ActuatorState::STOPPED:
                // No position change
                break;
        }
        lastTime = now; // Update lastTime only after processing the interval
    }
     // If dt is 0 or negative, do nothing - prevents weird calculations
}

void DigitalPosFeedback::resetPosition() {
    currentPosition = 0.0f;
    lastTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f; // Reset timer baseline
}

float DigitalPosFeedback::getEstimatedPosition() {
    return currentPosition;
}