#include "mbed.h"
#include <cstdio>
#include <iostream>
#include <Eigen/Dense>
#include <array>

using namespace std;
using namespace Eigen;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*Function Declarations*/
Matrix3f getRotationMatrix(float roll_deg, float pitch_deg, float yaw_deg);

int main()
{
   /*Defining the Base and Platform attachment point*/

   // Base attachment points (fixed)
    array<Vector3f, 6> base_joints = {
        Eigen::Vector3f(-50.0000f,   367.4546f, 0.0f),
        Eigen::Vector3f( 50.0000f,   367.4546f, 0.0f),
        Eigen::Vector3f(343.2250f,  -140.4260f, 0.0f),
        Eigen::Vector3f(293.2250f,  -227.0286f, 0.0f),
        Eigen::Vector3f(-293.2250f, -227.0286f, 0.0f),
        Eigen::Vector3f(-343.2250f, -140.4260f, 0.0f)
    };

    // Platform attachment points (home position, unrotated)
    array<Vector3f, 6> platform_joints_home = {
        Eigen::Vector3f(  50.0000f,  -286.1637f, 458.5300f),
        Eigen::Vector3f( -50.0000f,  -286.1637f, 458.5300f),
        Eigen::Vector3f(-272.8250f,    99.7806f, 458.5300f),
        Eigen::Vector3f(-222.8250f,   186.3831f, 458.5300f),
        Eigen::Vector3f( 222.8250f,   186.3831f, 458.5300f),
        Eigen::Vector3f( 272.8250f,    99.7806f, 458.5300f)
    };

    // Index mapping from MATLAB
    array<int, 6> base_idx  = {4, 3, 2, 1, 0, 5};  // corresponds to MATLAB [5 4 3 2 1 6]
    array<int, 6> plat_idx  = {1, 0, 5, 4, 3, 2};  // corresponds to MATLAB [2 1 6 5 4 3]


    // Platform Pose Input - for debugging only, these values will be replaced by Flight data from the MSFS2020
    float translationX = 0.0f;
    float translationY = 0.0f;
    float translationZ = 0.0f;

   
    float roll_deg  = 0.0f;
    float pitch_deg = 0.0f;
    float yaw_deg   = 0.0f;
    

    while (true) {

        std::cout << "\033[2J\033[H";

        Matrix3f R = getRotationMatrix(roll_deg, pitch_deg, yaw_deg);
        Vector3f translation_mm(translationX, translationY, translationZ);

        //cout << R << endl << endl;


        // Compute Platform Center (average of all points)
        Vector3f center_P = Vector3f::Zero();
        for (const auto& pt : platform_joints_home) {
            center_P += pt;
        }
        center_P /= platform_joints_home.size();

        // Transform Platform joints to world frame
        array<Vector3f, 6> platform_joints_world;                                                //The coordinates of the platform attachment points after translation and rotation
        for (size_t i = 0; i < platform_joints_home.size(); ++i) {
            platform_joints_world[i] = R * (platform_joints_home[i] - center_P) + center_P + translation_mm;
        }

        // Compute Actuator vectors and lengths
        array<Vector3f, 6> actuator_vectors;
        array<float, 6> actuator_lengths;

        for (size_t i = 0; i < 6; ++i) {
        actuator_vectors[i] = platform_joints_world[plat_idx[i]] - base_joints[base_idx[i]];
        actuator_lengths[i] = actuator_vectors[i].norm();

        }

        // cout << "Platform Joints (World Frame):\n";
        // for (size_t i = 0; i < 6; ++i) {
        //     cout << "P" << i + 1 << ": ["
        //         << platform_joints_world[i].x() << ", "
        //         << platform_joints_world[i].y() << ", "
        //         << platform_joints_world[i].z() << "] mm" << endl;
        // }

        // // Debug print actuator vectors
        // cout << "Actuator Vectors (Platform - Base):\n";
        // for (size_t i = 0; i < 6; ++i) {
        //     cout << "A" << i + 1 << ": ["
        //         << actuator_vectors[i].x() << ", "
        //         << actuator_vectors[i].y() << ", "
        //         << actuator_vectors[i].z() << "]" << endl;
        // }

        //Debug print lengths
        cout << "Actuator lengths:\n";   
        for (size_t i = 0; i < 6; ++i) {
            cout << "A" << i+1 << ": " << actuator_lengths[i] << " mm" << endl;
        }


        ThisThread::sleep_for(1000ms);
    }
}   


/*Function to Compute the Rotation Matrix*/
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
