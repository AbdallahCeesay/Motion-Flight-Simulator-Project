#include <mbed.h>
#include <BNO080.h>
#include <iostream>
#include <SerialStream.h>

#define i2cadd 0x4A    //I2C Address
#define i2cportspeed 400000

// defining hardware pins
PinName SCL = PB_10;
PinName SDA = PB_11;
PinName INTPin = PE_15;
PinName RSTPin = PE_14;

BufferedSerial serial(USBTX, USBRX, 115200);
SerialStream<BufferedSerial> debugport(serial);
BNO080I2C imu(&debugport, SDA, SCL, INTPin, RSTPin, i2cadd, i2cportspeed);


int main() {

    debugport.printf("============================================\n");

    if(imu.begin()) {

        printf("Initialization Success!\n"); // check that the initialization succeeded
        //imu.setSensorOrientation(Quaternion orientation)  // sets the IMU coordinate frame to the Platform' coordinate frame
    }

    else printf("Initialization Failed!\n");


    // Configure IMU reports
    imu.enableReport(BNO080::GAME_ROTATION, 20);

    while (true) {

       // std::cout << "\033[2J\033[H";       
         
        if (imu.updateData()) {

            if (imu.hasNewData(BNO080::GAME_ROTATION)) {
                
                TVector3 eulerDegrees = imu.gameRotationVector.euler() * (180.0 / M_PI);
                float roll = eulerDegrees[0];               //x-axis
                float pitch = eulerDegrees[1];              //y-axis
                float yaw = eulerDegrees[2];                //z-axis

                debugport.printf("Roll: %.2f\n", roll);
                debugport.printf("Pitch: %.2f\n", pitch);
                debugport.printf("Yaw: %.2f\n", yaw);
                debugport.printf("\n");
            }
        }

       ThisThread::sleep_for(2ms);eee
    }
}
