#include "DigiPosFeedback.hpp"
#include <iostream>
#include <iomanip>
using namespace std::chrono;

DigitalPosFeedback::DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed, float duty)
    : RPWM(rpwm), LPWM(lpwm), ACTUATOR_SPEED(actuatorspeed), DUTY_CYCLE(duty)
{
    timer.start();
    lastTime = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;
}

void DigitalPosFeedback::setDuty_Cycle(float duty) {
    DUTY_CYCLE = duty;
}

void DigitalPosFeedback::setActuatorSpeed(float speed) {
    ACTUATOR_SPEED = speed;
}

void DigitalPosFeedback::printPosition(const char* label) {
    std::cout << label << " Position: " << std::fixed << std::setprecision(2) << currentPosition << " mm\n";
}

void DigitalPosFeedback::updateStateFromPWM() {
    if (RPWM.read() > 0.0f && LPWM.read() == 0.0f) {
        state = ActuatorState::RETRACTING;
    } else if (LPWM.read() > 0.0f && RPWM.read() == 0.0f) {
        state = ActuatorState::EXTENDING;
    } else {
        state = ActuatorState::STOPPED;
    }
}

void DigitalPosFeedback::updatePosition() {
    updateStateFromPWM();                   // Sync state based on current PWM pin output

    float now = duration_cast<milliseconds>(timer.elapsed_time()).count() / 1000.0f;
    float dt = now - lastTime;

    if (dt > 0) {
        switch (state) {
            case ActuatorState::EXTENDING:
                currentPosition += ACTUATOR_SPEED * dt;
                if (currentPosition > MAX_STROKE)
                    currentPosition = MAX_STROKE;
                break;

            case ActuatorState::RETRACTING:
                currentPosition -= ACTUATOR_SPEED * dt;
                if (currentPosition < 0.0f)
                    currentPosition = 0.0f;
                break;

            case ActuatorState::STOPPED:
                // no position change
                break;
        }

        lastTime = now;
    }
}

void DigitalPosFeedback::extend() {
    LPWM.write(DUTY_CYCLE);
    RPWM.write(0.0f);
}

void DigitalPosFeedback::retract() {
    RPWM.write(DUTY_CYCLE);
    LPWM.write(0.0f);
}

void DigitalPosFeedback::stop() {
    RPWM.write(0.0f);
    LPWM.write(0.0f);
}

void DigitalPosFeedback::moveToTarget() {
    updatePosition();  // update the current position

    float error = targetPosition - currentPosition;

    if (fabs(error) < tolerance) {
        stop();
    } else if (error > 0) {
        extend();
    } else {
        retract();
    }
}

