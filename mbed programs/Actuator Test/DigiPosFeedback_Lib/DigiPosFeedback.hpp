
#ifndef DIGI_POS_FEEDBACK_HPP
#define DIGI_POS_FEEDBACK_HPP

#include "mbed.h"
#include <chrono>

class DigitalPosFeedback {
    private:
        float DUTY_CYCLE;
        float ACTUATOR_SPEED;
        static constexpr float MAX_STROKE = 300.0f;

        Timer timer;
        float lastTime;

        PwmOut RPWM;
        PwmOut LPWM;

    public:
        float currentPosition;

        enum class ActuatorState {
            EXTENDING,
            RETRACTING,
            STOPPED
        };

        ActuatorState state;

        DigitalPosFeedback(PinName rpwm, PinName lpwm, float actuatorspeed = 30.6827057f, float duty = 1.0f);

        void setDuty_Cycle(float speed);
        void setActuatorSpeed(float speed);
        void handleInput(char c);
        void updatePosition();
        void printPosition(const char* label);
};

#endif 