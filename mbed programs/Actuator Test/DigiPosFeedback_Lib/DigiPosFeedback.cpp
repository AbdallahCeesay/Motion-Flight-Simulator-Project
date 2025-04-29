#include "DigiPosFeedback_Lib/DigiPosFeedback.hpp"
#include <iostream>
#include <iomanip>

using namespace std::chrono;

DigitalPosFeedback::DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed, float duty)
    : RPWM(rpwm), LPWM(lpwm), ACTUATOR_SPEED(actuatorspeed), DUTY_CYCLE(duty), currentPosition(0.0f), state(ActuatorState::STOPPED)
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

void DigitalPosFeedback::handleInput(char c) {
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
            break;
    }
}

void DigitalPosFeedback::updatePosition() {
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
}