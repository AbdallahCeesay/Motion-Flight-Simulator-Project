#include "mbed.h"
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <Eigen/Dense>
#include <array>
#include "DigiPosFeedback_Lib/DigiPosFeedback.hpp"

using namespace std;
using namespace Eigen;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*Function Declarations*/
Matrix3f getRotationMatrix(float roll_deg, float pitch_deg, float yaw_deg);

int main()
{


    // Defining Feedback Pins and Variables
    DigitalPosFeedback actuator1(PC_8, PC_9, 30.6827057f, 0.6f); 
    DigitalPosFeedback actuator2(PE_5, PE_6, 30.6827057f, 0.6f); 
    DigitalPosFeedback actuator3(PB_8, PB_9);
    DigitalPosFeedback actuator4(PA_5, PA_6);
    DigitalPosFeedback actuator5(PD_14, PD_15);
    DigitalPosFeedback actuator6(PE_9, PE_11);

    // Platform always starts at 0mm therefore current pos is initialised to 0mm
    actuator1.currentPosition = 100.0f;
    actuator2.currentPosition = 100.0f;
    actuator3.currentPosition = 100.0f;
    actuator4.currentPosition = 100.0f;
    actuator5.currentPosition = 100.0f;
    actuator6.currentPosition = 100.0f;

   /*Defining the Base and Platform attachment points*/
    array<Vector3f, 6> base_joints = {
        Vector3f(-50.0000f,   367.4546f, 0.0f),
        Vector3f( 50.0000f,   367.4546f, 0.0f),
        Vector3f(343.2250f,  -140.4260f, 0.0f),
        Vector3f(293.2250f,  -227.0286f, 0.0f),
        Vector3f(-293.2250f, -227.0286f, 0.0f),
        Vector3f(-343.2250f, -140.4260f, 0.0f)
    };

    array<Vector3f, 6> platform_joints_home = {
        Vector3f(  50.0000f,  -286.1637f, 458.5300f),
        Vector3f( -50.0000f,  -286.1637f, 458.5300f),
        Vector3f(-272.8250f,    99.7806f, 458.5300f),
        Vector3f(-222.8250f,   186.3831f, 458.5300f),
        Vector3f( 222.8250f,   186.3831f, 458.5300f),
        Vector3f( 272.8250f,    99.7806f, 458.5300f)
    };

    array<int, 6> base_idx  = {4, 3, 2, 1, 0, 5};  
    array<int, 6> plat_idx  = {1, 0, 5, 4, 3, 2};  
    //array<string, 6> matlab_labels = {"A1", "A2", "A3", "A4", "A5", "A6"};

    // Platform Pose Input
    float translationX = 0.0f;
    float translationY = 0.0f;
    float translationZ = 0.0f;


    float roll_deg  = 0.0f;
    float pitch_deg = 30.0f;
    float yaw_deg   = 0.0f;

   

    // Continuous control loop
    while (true) {

        std::cout << "\033[2J\033[H";

         // Calculate actuator lengths once based on pose
        Matrix3f R = getRotationMatrix(roll_deg, pitch_deg, yaw_deg);
        Vector3f translation_mm(translationX, translationY, translationZ);

        Vector3f center_P = Vector3f::Zero();
        for (const auto& pt : platform_joints_home) {
            center_P += pt;
        }
        center_P /= platform_joints_home.size();

        array<Vector3f, 6> platform_joints_world;
        for (size_t i = 0; i < platform_joints_home.size(); ++i) {
            platform_joints_world[i] = R * (platform_joints_home[i] - center_P) + center_P + translation_mm;
        }

        array<float, 6> actuator_lengths;
        for (size_t i = 0; i < 6; ++i) {
            Vector3f actuator_vector = platform_joints_world[plat_idx[i]] - base_joints[base_idx[i]];
            actuator_lengths[i] = actuator_vector.norm();
        }

        // Assign actuator targets
        actuator1.targetPosition = actuator_lengths[4]; 
        actuator2.targetPosition = actuator_lengths[3]; 
        actuator3.targetPosition = actuator_lengths[2]; 
        actuator4.targetPosition = actuator_lengths[1]; 
        actuator5.targetPosition = actuator_lengths[0]; 
        actuator6.targetPosition = actuator_lengths[5]; 

        actuator5.extend();
        wait_us(5000000);
        actuator5.retract();
        wait_us(5000000);

        cout << "A1 (actuator1): " << actuator1.targetPosition << " mm\n";
        cout << "A2 (actuator2): " << actuator2.targetPosition << " mm\n";
        cout << "A3 (actuator3): " << actuator3.targetPosition << " mm\n";
        cout << "A4 (actuator4): " << actuator4.targetPosition << " mm\n";
        cout << "A5 (actuator5): " << actuator5.targetPosition << " mm\n";
        cout << "A6 (actuator6): " << actuator6.targetPosition << " mm\n";



        ThisThread::sleep_for(1000ms);  // Smooth 100Hz control
    }
}

/* Function to Compute the Rotation Matrix */
Matrix3f getRotationMatrix(float roll_deg, float pitch_deg, float yaw_deg) {
    float roll  = roll_deg  * M_PI / 180.0f;
    float pitch = pitch_deg * M_PI / 180.0f;
    float yaw   = yaw_deg   * M_PI / 180.0f;

    Matrix3f Rx;
    Rx << 1,       0,        0,
          0, cos(roll), -sin(roll),
          0, sin(roll),  cos(roll);

    Matrix3f Ry;
    Ry << cos(pitch), 0, sin(pitch),
          0,          1, 0,
         -sin(pitch), 0, cos(pitch);

    Matrix3f Rz;
    Rz << cos(yaw), -sin(yaw), 0,
          sin(yaw),  cos(yaw), 0,
               0,         0,   1;

    return Ry * Rz * Rx;
}
