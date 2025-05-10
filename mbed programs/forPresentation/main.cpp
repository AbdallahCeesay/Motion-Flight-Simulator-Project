#include "mbed.h"
#include <cstdio> // Use cstdio for printf
#include "DigiPosFeedback_Lib/DigiPosFeedback.hpp"
#include <vector> // To hold actuator pointers
#include <chrono> // For durations
// #include "mbed_pinmap.h" // <--- REMOVE THIS INCLUDE

using namespace std;
using namespace std::chrono;

// LED indicators
DigitalOut led1(LED1); // Indicate running sequence
DigitalOut led2(LED2); // Indicate idle/stopped

// Serial communication
static BufferedSerial terminal(USBTX, USBRX, 115200);

// --- Configuration ---
const float ACTUATOR_SPEED = 30.6827057f; // mm/s
const float MAX_ACTUATOR_STROKE = 300.0f; // mm
const float ACTUATOR_DUTY_CYCLE = 0.6f;   // Default duty cycle

// Timing calculations
const float TIME_FULL_STROKE_S = MAX_ACTUATOR_STROKE / ACTUATOR_SPEED; // Approx 9.777 seconds
const milliseconds TIME_FOR_100MM = chrono::duration_cast<milliseconds>(chrono::duration<float>((100.0f / MAX_ACTUATOR_STROKE) * TIME_FULL_STROKE_S));
const milliseconds TIME_FOR_40_DEG = 3000ms; // Estimated time for 40deg rotation

// Durations for motion segments
const milliseconds INITIAL_EXTEND_DURATION = 3500ms;
const milliseconds ROTATION_DURATION_HALF = TIME_FOR_40_DEG;
const milliseconds TRANSLATION_DURATION_HALF = TIME_FOR_100MM;
const milliseconds PAUSE_DURATION = 1000ms;
const milliseconds HOMING_DURATION = chrono::duration_cast<milliseconds>(chrono::duration<float>(TIME_FULL_STROKE_S * 1.1f));


// --- Actuator Setup ---
std::vector<DigitalPosFeedback*> actuators;

// --- State Machine ---
enum class DemoState {
    IDLE,
    RUNNING,
    STOPPING,
    HOMING
};
DemoState currentState = DemoState::IDLE;
bool sequenceComplete = false;


// --- Helper Functions (stopAllActuators, homePlatform, printStatus - unchanged) ---
// Stop all actuators
void stopAllActuators() {
    for (auto& act : actuators) {
        act->stop();
    }
}

// Retract all actuators for homing
void homePlatform() {
    printf("Homing: Retracting all actuators...\n");
    for (auto& act : actuators) {
        act->retract();
    }
    ThisThread::sleep_for(HOMING_DURATION); // Wait for retraction
    stopAllActuators();
    for (auto& act : actuators) { // Reset estimated position after homing
        act->resetPosition();
    }
    printf("Homing complete (estimated).\n");
}

// Print current status
void printStatus() {
    // Clear screen and move cursor to home
    printf("\033[2J\033[H");
    printf("--- Stewart Platform Demo ---\n");
    printf("State: ");
    switch (currentState) {
        case DemoState::IDLE:     printf("IDLE (Press 'e' to start, 'q' to home)\n"); break;
        case DemoState::RUNNING:  printf("RUNNING SEQUENCE (Press 's' to stop, 'q' to home)\n"); break;
        case DemoState::STOPPING: printf("STOPPING...\n"); break;
        case DemoState::HOMING:   printf("HOMING...\n"); break;
    }
    printf("Target Ranges: +/- %.0f deg, +/- %.0f mm\n", 40.0, 100.0);
    printf("-----------------------------\n");
    for (size_t i = 0; i < actuators.size(); ++i) {
        char label[20];
        snprintf(label, sizeof(label), "Actuator %zu", i + 1); // Show A1..A6
        actuators[i]->printPosition(label);
    }
    printf("-----------------------------\n");
    printf("Note: Platform may not remain perfectly flat during pure translation\n");
    printf("      due to open-loop control and actuator mechanics.\n");
    fflush(stdout); // Make sure output is sent
}


// --- Main Program ---
int main()
{
    terminal.set_blocking(false);

    printf("--- Mbed Stewart Platform Controller Initializing ---\n");
    printf("Full stroke time: %.2f s\n", TIME_FULL_STROKE_S);
    printf("Time for 100mm (half trans): %lld ms\n", TRANSLATION_DURATION_HALF.count());
    printf("Time for 40 deg (half rot): %lld ms (estimated)\n", ROTATION_DURATION_HALF.count());
    printf("Actuator Mapping (Check Wiring):\n");
    // *** CHANGE HERE: Use string literals for pin names ***
    printf("  Actuator 1 (A1 - %s,%s)\n", "PC_8", "PC_9");
    printf("  Actuator 2 (A2 - %s,%s)\n", "PE_5", "PE_6");
    printf("  Actuator 3 (A3 - %s,%s)\n", "PB_8", "PB_9");
    printf("  Actuator 4 (A4 - %s,%s)\n", "PA_5", "PA_6");
    printf("  Actuator 5 (A5 - %s,%s)\n", "PD_13", "PD_12");
    printf("  Actuator 6 (A6 - %s,%s)\n", "PE_9", "PE_11");
    printf("Press 'e' to start, 's' to stop, 'q' to home.\n");


    // Initialize Actuators
    actuators.push_back(new DigitalPosFeedback(PC_8, PC_9, ACTUATOR_SPEED, ACTUATOR_DUTY_CYCLE)); // Actuator 1 (A1)
    actuators.push_back(new DigitalPosFeedback(PE_5, PE_6, ACTUATOR_SPEED, ACTUATOR_DUTY_CYCLE)); // Actuator 2 (A2)
    actuators.push_back(new DigitalPosFeedback(PB_8, PB_9, ACTUATOR_SPEED, ACTUATOR_DUTY_CYCLE)); // Actuator 3 (A3)
    actuators.push_back(new DigitalPosFeedback(PA_5, PA_6, ACTUATOR_SPEED, ACTUATOR_DUTY_CYCLE)); // Actuator 4 (A4)
    actuators.push_back(new DigitalPosFeedback(PD_13, PD_12, ACTUATOR_SPEED, ACTUATOR_DUTY_CYCLE)); // Actuator 5 (A5)
    actuators.push_back(new DigitalPosFeedback(PE_9, PE_11, ACTUATOR_SPEED, ACTUATOR_DUTY_CYCLE)); // Actuator 6 (A6)

    if (actuators.size() != 6) {
        printf("Error: Failed to initialize all actuators!\n");
        led1 = 1; led2 = 1; return 1;
    }

    stopAllActuators();
     for (auto& act : actuators) act->resetPosition();

    currentState = DemoState::IDLE;
    led1 = 0; led2 = 1;

    while (true) {
        // --- Input Handling (unchanged) ---
        char inputChar = '\0';
        if (terminal.readable()) {
            ssize_t numRead = terminal.read(&inputChar, 1);
            if (numRead > 0) {
                 if (inputChar == 'e' && currentState == DemoState::IDLE) {
                    currentState = DemoState::RUNNING; sequenceComplete = false;
                    printf("Starting sequence...\n"); led1 = 1; led2 = 0;
                } else if (inputChar == 's') {
                    if (currentState == DemoState::RUNNING || currentState == DemoState::HOMING) {
                         currentState = DemoState::STOPPING; printf("Stop requested.\n");
                    } else {
                         stopAllActuators(); currentState = DemoState::IDLE; led1 = 0; led2 = 1;
                    }
                } else if (inputChar == 'q') {
                     if (currentState != DemoState::HOMING) {
                        currentState = DemoState::HOMING; printf("Home requested.\n"); led1 = 1; led2 = 1;
                     }
                }
            }
        }


        // --- State Machine Logic (unchanged from previous version) ---
        switch (currentState) {
            case DemoState::IDLE:
                led1 = 0; led2 = 1;
                break;

            case DemoState::RUNNING:
                if (!sequenceComplete) {
                    printf("\n--- SEQUENCE START ---\n");

                    // Steps 1-10 (Initial Extend, Roll, Pitch, Yaw) are unchanged...
                    // 1. Initial Extend
                    printf("Step 1: Initial Extend (%lld ms)\n", INITIAL_EXTEND_DURATION.count());
                    for(auto& act : actuators) act->extend();
                    ThisThread::sleep_for(INITIAL_EXTEND_DURATION); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 2. Roll Left
                    printf("Step 2: Roll Left (+40 deg, %lld ms)\n", ROTATION_DURATION_HALF.count());
                    actuators[1]->extend(); actuators[2]->extend(); actuators[4]->retract(); actuators[5]->retract(); actuators[0]->stop(); actuators[3]->stop();
                    ThisThread::sleep_for(ROTATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 3. Roll Right
                    printf("Step 3: Roll Right (-40 deg, %lld ms)\n", (ROTATION_DURATION_HALF * 2).count());
                    actuators[4]->extend(); actuators[5]->extend(); actuators[1]->retract(); actuators[2]->retract(); actuators[0]->stop(); actuators[3]->stop();
                    ThisThread::sleep_for(ROTATION_DURATION_HALF * 2); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 4. Roll Center
                    printf("Step 4: Roll Center (from -40 deg, %lld ms)\n", ROTATION_DURATION_HALF.count());
                    actuators[1]->extend(); actuators[2]->extend(); actuators[4]->retract(); actuators[5]->retract(); actuators[0]->stop(); actuators[3]->stop();
                    ThisThread::sleep_for(ROTATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 5. Pitch Up
                    printf("Step 5: Pitch Up (+40 deg, %lld ms)\n", ROTATION_DURATION_HALF.count());
                    actuators[0]->extend(); actuators[5]->extend(); actuators[2]->retract(); actuators[3]->retract(); actuators[1]->stop(); actuators[4]->stop();
                    ThisThread::sleep_for(ROTATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 6. Pitch Down
                    printf("Step 6: Pitch Down (-40 deg, %lld ms)\n", (ROTATION_DURATION_HALF * 2).count());
                    actuators[2]->extend(); actuators[3]->extend(); actuators[0]->retract(); actuators[5]->retract(); actuators[1]->stop(); actuators[4]->stop();
                    ThisThread::sleep_for(ROTATION_DURATION_HALF * 2); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 7. Pitch Center
                    printf("Step 7: Pitch Center (from -40 deg, %lld ms)\n", ROTATION_DURATION_HALF.count());
                    actuators[0]->extend(); actuators[5]->extend(); actuators[2]->retract(); actuators[3]->retract(); actuators[1]->stop(); actuators[4]->stop();
                    ThisThread::sleep_for(ROTATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 8. Yaw Left
                    printf("Step 8: Yaw Left (+40 deg, %lld ms)\n", ROTATION_DURATION_HALF.count());
                    actuators[0]->extend(); actuators[2]->extend(); actuators[4]->extend(); actuators[1]->retract(); actuators[3]->retract(); actuators[5]->retract();
                    ThisThread::sleep_for(ROTATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 9. Yaw Right
                    printf("Step 9: Yaw Right (-40 deg, %lld ms)\n", (ROTATION_DURATION_HALF * 2).count());
                    actuators[1]->extend(); actuators[3]->extend(); actuators[5]->extend(); actuators[0]->retract(); actuators[2]->retract(); actuators[4]->retract();
                    ThisThread::sleep_for(ROTATION_DURATION_HALF * 2); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 10. Yaw Center
                    printf("Step 10: Yaw Center (from -40 deg, %lld ms)\n", ROTATION_DURATION_HALF.count());
                    actuators[0]->extend(); actuators[2]->extend(); actuators[4]->extend(); actuators[1]->retract(); actuators[3]->retract(); actuators[5]->retract();
                    ThisThread::sleep_for(ROTATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 11. Translate X Right
                    printf("Step 11: Translate X Right (+100mm, %lld ms)\n", TRANSLATION_DURATION_HALF.count());
                    actuators[1]->extend(); actuators[2]->extend();   actuators[4]->retract(); actuators[5]->retract(); actuators[0]->stop(); actuators[3]->stop();
                    ThisThread::sleep_for(TRANSLATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 12. Translate X Left
                    printf("Step 12: Translate X Left (-100mm, %lld ms)\n", (TRANSLATION_DURATION_HALF * 2).count());
                    actuators[1]->retract(); actuators[2]->retract(); actuators[4]->extend(); actuators[5]->extend();   actuators[0]->stop(); actuators[3]->stop();
                    ThisThread::sleep_for(TRANSLATION_DURATION_HALF * 2); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 13. Translate X Center
                    printf("Step 13: Translate X Center (from -100mm, %lld ms)\n", TRANSLATION_DURATION_HALF.count());
                    actuators[1]->extend(); actuators[2]->extend();   actuators[4]->retract(); actuators[5]->retract(); actuators[0]->stop(); actuators[3]->stop();
                    ThisThread::sleep_for(TRANSLATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 14. Translate Y Forward
                    printf("Step 14: Translate Y Forward (+100mm, %lld ms)\n", TRANSLATION_DURATION_HALF.count());
                    actuators[0]->extend(); actuators[3]->extend();   actuators[2]->retract(); actuators[5]->retract(); actuators[1]->stop(); actuators[4]->stop();
                    ThisThread::sleep_for(TRANSLATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 15. Translate Y Backward
                    printf("Step 15: Translate Y Backward (-100mm, %lld ms)\n", (TRANSLATION_DURATION_HALF * 2).count());
                    actuators[0]->retract(); actuators[3]->retract(); actuators[2]->extend(); actuators[5]->extend();   actuators[1]->stop(); actuators[4]->stop();
                    ThisThread::sleep_for(TRANSLATION_DURATION_HALF * 2); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 16. Translate Y Center
                    printf("Step 16: Translate Y Center (from -100mm, %lld ms)\n", TRANSLATION_DURATION_HALF.count());
                    actuators[0]->extend(); actuators[3]->extend();   actuators[2]->retract(); actuators[5]->retract(); actuators[1]->stop(); actuators[4]->stop();
                    ThisThread::sleep_for(TRANSLATION_DURATION_HALF); stopAllActuators(); ThisThread::sleep_for(PAUSE_DURATION); if (currentState != DemoState::RUNNING) break;
                    // 17. Return Home
                    printf("Step 17: Returning Home...\n");
                    homePlatform();

                    printf("--- SEQUENCE COMPLETE ---\n");
                    sequenceComplete = true;
                    currentState = DemoState::IDLE;
                    led1 = 0; led2 = 1;
                }
                break; // End of RUNNING case

            case DemoState::STOPPING:
                stopAllActuators();
                currentState = DemoState::IDLE;
                led1 = 0; led2 = 1;
                break;

            case DemoState::HOMING:
                homePlatform();
                currentState = DemoState::IDLE;
                led1 = 0; led2 = 1;
                break;
        }

        // --- Update and Print Status ---
        for (auto& act : actuators) {
            act->updatePosition();
        }
        printStatus();

        // Main loop delay
        ThisThread::sleep_for(50ms);
    }
    // return 0; // Unreachable
}