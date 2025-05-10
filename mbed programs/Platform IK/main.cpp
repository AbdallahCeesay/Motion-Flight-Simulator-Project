#include "mbed.h"
#include <cstdio>
// #include <iostream> // std::cout not strictly needed if using printf
#include <iomanip>
#include <Eigen/Dense>
#include <array>
#include "DigiPosFeedback_Lib/DigiPosFeedback.hpp" // Correct path assumed

using namespace std; // For std::array, std::pair etc.
using namespace Eigen;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Physical Constants ---
// !!! VERIFIED VALUE FOR YOUR ACTUATORS !!!
const float BASE_ACTUATOR_LENGTH = 500.0f; // Minimum length (mm) when stroke is 0 (including joints)
const float ACTUATOR_SPEED_MM_PER_S = 30.6827057f;
// Default duty cycles will be set individually below
const float CONTROL_LOOP_PERIOD_MS = 20; // Control loop frequency (50 Hz)
const float INITIAL_ACTUATOR_STROKE = 100.0f; // Initial stroke position (mm)

// --- Function Declarations ---
Matrix3f getRotationMatrix(float roll_deg, float pitch_deg, float yaw_deg);

int main()
{
    // Use BufferedSerial for potentially non-blocking output IF NEEDED LATER
    // BufferedSerial pc(USBTX, USBRX, 115200); // Adjust baud rate if needed

    // REMOVED: cout.rdbuf(pc.rdbuf()); // Redirect std::cout - Not the standard Mbed OS 6 way
                                        // printf() will output to console via USBTX/USBRX by default

    printf("--- Stewart Platform Control Initializing ---\n");
    printf("Base Actuator Length (Retracted + Joints): %.2f mm\n", BASE_ACTUATOR_LENGTH);

    // --- Define Actuators ---
    // Use std::array for easier management
    // Initialize with default speed, duty cycle will be set individually below
    array<DigitalPosFeedback, 6> actuators = {
        DigitalPosFeedback(PC_8, PC_9, ACTUATOR_SPEED_MM_PER_S),     // Actuator 1 (Connects B5-P2 in MATLAB -> A1 label)
        DigitalPosFeedback(PE_5, PE_6, ACTUATOR_SPEED_MM_PER_S),     // Actuator 2 (Connects B4-P1 in MATLAB -> A2 label)
        DigitalPosFeedback(PB_8, PB_9, ACTUATOR_SPEED_MM_PER_S),     // Actuator 3 (Connects B3-P6 in MATLAB -> A3 label)
        DigitalPosFeedback(PA_5, PA_6, ACTUATOR_SPEED_MM_PER_S),     // Actuator 4 (Connects B2-P5 in MATLAB -> A4 label)
        DigitalPosFeedback(PD_13, PD_12, ACTUATOR_SPEED_MM_PER_S),   // Actuator 5 (Connects B1-P4 in MATLAB -> A5 label)
        DigitalPosFeedback(PE_9, PE_11, ACTUATOR_SPEED_MM_PER_S)     // Actuator 6 (Connects B6-P3 in MATLAB -> A6 label)
    };

    // Set initial estimated stroke position and specific duty cycles for all actuators
    actuators[0].setDuty_Cycle(0.6f); // Actuator 1 duty cycle
    actuators[1].setDuty_Cycle(0.6f); // Actuator 2 duty cycle
    actuators[2].setDuty_Cycle(1.0f); // Actuator 3 duty cycle
    actuators[3].setDuty_Cycle(1.0f); // Actuator 4 duty cycle
    actuators[4].setDuty_Cycle(1.0f); // Actuator 5 duty cycle
    actuators[5].setDuty_Cycle(1.0f); // Actuator 6 duty cycle

    printf("Actuator Duty Cycles: A1=%.1f, A2=%.1f, A3=%.1f, A4=%.1f, A5=%.1f, A6=%.1f\n",
           actuators[0].getDutyCycle(), actuators[1].getDutyCycle(), actuators[2].getDutyCycle(),
           actuators[3].getDutyCycle(), actuators[4].getDutyCycle(), actuators[5].getDutyCycle());


    for (auto& act : actuators) {
        act.currentPosition = INITIAL_ACTUATOR_STROKE;
        act.tolerance = 15.0f; // Tolerance for position error (adjust as needed)
    }
    printf("Actuators initialized. Initial stroke set to %.2f mm.\n", INITIAL_ACTUATOR_STROKE);


    // --- Define Base and Platform Geometry (Matches MATLAB Output) ---
    const array<Vector3f, 6> base_joints = {
        /*b1*/ Vector3f(-293.2250f, -227.0286f, 0.0f), // Index 0
        /*b2*/ Vector3f( 293.2250f, -227.0286f, 0.0f), // Index 1
        /*b3*/ Vector3f( 343.2250f, -140.4260f, 0.0f), // Index 2
        /*b4*/ Vector3f(  50.0000f,  367.4546f, 0.0f), // Index 3
        /*b5*/ Vector3f( -50.0000f,  367.4546f, 0.0f), // Index 4
        /*b6*/ Vector3f(-343.2250f, -140.4260f, 0.0f)  // Index 5
    };

    // Platform joints defined at their 'home' pose (after 180-degree rotation from MATLAB script)
    const array<Vector3f, 6> platform_joints_home_rotated = {
        /*p1*/ Vector3f( -50.0000f,  -286.1637f, 458.5300f), // Index 0
        /*p2*/ Vector3f(  50.0000f,  -286.1637f, 458.5300f), // Index 1
        /*p3*/ Vector3f( 272.8250f,    99.7806f, 458.5300f), // Index 2
        /*p4*/ Vector3f( 222.8250f,   186.3831f, 458.5300f), // Index 3
        /*p5*/ Vector3f(-222.8250f,   186.3831f, 458.5300f), // Index 4
        /*p6*/ Vector3f(-272.8250f,    99.7806f, 458.5300f)  // Index 5
    };

    // --- Define Actuator Connectivity (Indices for base_joints and platform_joints_home_rotated) ---
    // Maps Actuator Index (0-5 corresponding to actuators array) to Base/Platform Joint Indices
    const array<pair<int, int>, 6> actuator_connections = {
        /* Actuator 0 (A1) */ make_pair(4, 1), // Base joint b5 (index 4), Platform joint p2 (index 1)
        /* Actuator 1 (A2) */ make_pair(3, 0), // Base joint b4 (index 3), Platform joint p1 (index 0)
        /* Actuator 2 (A3) */ make_pair(2, 5), // Base joint b3 (index 2), Platform joint p6 (index 5)
        /* Actuator 3 (A4) */ make_pair(1, 4), // Base joint b2 (index 1), Platform joint p5 (index 4)
        /* Actuator 4 (A5) */ make_pair(0, 3), // Base joint b1 (index 0), Platform joint p4 (index 3)
        /* Actuator 5 (A6) */ make_pair(5, 2)  // Base joint b6 (index 5), Platform joint p3 (index 2)
    };


    // --- Platform Pose Input (Example: Pitch 30 degrees) ---
    float translationX_mm = 0.0f;
    float translationY_mm = 0.0f;
    float translationZ_mm = 0.0f;
    float roll_deg  = 0.0f;
    float pitch_deg = 30.0f;
    float yaw_deg   = 0.0f;

    printf("Target Pose: T=[%.1f, %.1f, %.1f] mm, RPY=[%.1f, %.1f, %.1f] deg\n",
           translationX_mm, translationY_mm, translationZ_mm, roll_deg, pitch_deg, yaw_deg);


    // --- Pre-calculate Platform Home Center ---
    Vector3f center_P_home = Vector3f::Zero();
    for (const auto& pt : platform_joints_home_rotated) {
        center_P_home += pt;
    }
    center_P_home /= platform_joints_home_rotated.size();

    printf("--- Starting Control Loop ---\n");

    // --- Continuous Control Loop ---
    while (true) {
        // 1. Calculate Target Pose Transformation
        Matrix3f R = getRotationMatrix(roll_deg, pitch_deg, yaw_deg);
        Vector3f T(translationX_mm, translationY_mm, translationZ_mm);

        // 2. Calculate Transformed Platform Joint Positions in World Frame
        array<Vector3f, 6> platform_joints_world;
        for (size_t i = 0; i < platform_joints_home_rotated.size(); ++i) {
            platform_joints_world[i] = R * (platform_joints_home_rotated[i] - center_P_home) + center_P_home + T;
        }

        // 3. Calculate Required TOTAL Actuator Lengths (Base Joint to Platform Joint)
        array<float, 6> target_total_lengths;
        for (size_t i = 0; i < 6; ++i) { // Loop through actuators 0 to 5
            int base_idx = actuator_connections[i].first;
            int plat_idx = actuator_connections[i].second;
            Vector3f actuator_vector = platform_joints_world[plat_idx] - base_joints[base_idx];
            target_total_lengths[i] = actuator_vector.norm();
        }

        // 4. Convert Total Lengths to Target STROKES and Update Actuator Targets
        for (size_t i = 0; i < 6; ++i) { // Loop through actuators 0 to 5
            // Target stroke = Total required length - Length when stroke is zero
            float target_stroke = target_total_lengths[i] - BASE_ACTUATOR_LENGTH;

            // Clamp target stroke to valid physical range [0, MAX_STROKE]
            // Note: Using actuators[i].MAX_STROKE allows potential future flexibility
            // if MAX_STROKE were not static constexpr, but here it refers to the static const.
            if (target_stroke < 0.0f) {
                target_stroke = 0.0f;
                 // Optional: Add a warning if commanded length is too short
                 // printf("Warning: Actuator %d target stroke clamped to 0 (demanded length %.2f mm too short)\n", i + 1, target_total_lengths[i]);
            } else if (target_stroke > DigitalPosFeedback::MAX_STROKE) { // Access static member
                target_stroke = DigitalPosFeedback::MAX_STROKE;
                 // Optional: Add a warning if commanded length is too long
                 // printf("Warning: Actuator %d target stroke clamped to %.2f (demanded length %.2f mm too long)\n", i + 1, DigitalPosFeedback::MAX_STROKE, target_total_lengths[i]);
            }

            actuators[i].targetPosition = target_stroke; // Set the target STROKE for the feedback controller
        }

        // 5. Update Position Estimates and Move Actuators Towards Target Stroke
        for (size_t i = 0; i < 6; ++i) {
            actuators[i].updatePosition(); // Update estimated position based on time and state
            actuators[i].moveToTarget();   // Command motion (extend/retract/stop) based on error
        }

        // --- Optional: Print Status periodically ---
        static LowPowerTimeout printTimer; // Use LowPowerTimeout for one-shot delay
        static bool canPrint = true;
        if (canPrint) {
             canPrint = false; // Prevent printing again until timer expires
             printTimer.attach([](){ canPrint = true; }, 1.0); // Re-enable printing after 1 second

             // Consider clearing screen less often if it causes flickering
             // printf("\033[2J\033[H"); // Clear screen

             printf("--- Status @ %llu ms ---\n", Kernel::Clock::now().time_since_epoch().count()); // Timestamp
             printf("Target Pose: T=[%.1f, %.1f, %.1f], RPY=[%.1f, %.1f, %.1f]\n",
                    translationX_mm, translationY_mm, translationZ_mm, roll_deg, pitch_deg, yaw_deg);
             for (size_t i = 0; i < 6; ++i) {
                 printf("Act %d: TargetLen=%.1f, TargetStroke=%.1f, CurrentStroke=%.1f, State=%d\n",
                        i + 1, // Use i+1 for 1-based actuator numbering in printout
                        target_total_lengths[i],
                        actuators[i].targetPosition,
                        actuators[i].currentPosition,
                        static_cast<int>(actuators[i].state));
             }
             printf("---------------------------\n");
         }
        // --- End Optional Print ---


        // Wait for next control cycle
        ThisThread::sleep_for(chrono::milliseconds(static_cast<long>(CONTROL_LOOP_PERIOD_MS)));
    }
}

/* Function to Compute the Rotation Matrix (Z-Y-X Order from MATLAB) */
Matrix3f getRotationMatrix(float roll_deg, float pitch_deg, float yaw_deg) {
    float roll  = roll_deg  * M_PI / 180.0f;
    float pitch = pitch_deg * M_PI / 180.0f;
    float yaw   = yaw_deg   * M_PI / 180.0f;

    Matrix3f Rx;
    Rx << 1,       0,        0,
          0, cosf(roll), -sinf(roll), // Use cosf/sinf for float
          0, sinf(roll),  cosf(roll);

    Matrix3f Ry;
    Ry << cosf(pitch), 0, sinf(pitch),
          0,           1, 0,
         -sinf(pitch), 0, cosf(pitch);

    Matrix3f Rz;
    Rz << cosf(yaw), -sinf(yaw), 0,
          sinf(yaw),  cosf(yaw), 0,
               0,          0,    1;

    // Apply in Z-Y-X order as per MATLAB calculation: P = R * (P_home - center_P)... where R = Ry * Rz * Rx;
    return Ry * Rz * Rx;
}