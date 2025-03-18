#include <mbed.h>
#include <BNO080.h>
#include <SerialStream.h>

#define i2cadd 0x4A    //I2C Address
#define i2cportspeed 400000

// defining hardware pins
PinName SDA = PB_9;
PinName SCL = PB_8;
PinName INTPin = PA_6;
PinName RSTPin = PA_5;

BufferedSerial serial(USBTX, USBRX, 115200);
SerialStream<BufferedSerial> debugport(serial);
BNO080I2C imu(&debugport, SDA, SCL, INTPin, RSTPin, i2cadd, i2cportspeed);


int main() {

    debugport.printf("============================================\n");

    if(imu.begin()) 
    printf("Init Success!\n"); // check that the initialization succeeded

    else printf("Init Failed!\n");

    // Configure IMU reports
    imu.enableReport(BNO080::ROTATION, 20);

    while (true) {
        
        if (imu.updateData()) {

            if (imu.hasNewData(BNO080::ROTATION)) {
                
                TVector3 eulerDegrees = imu.rotationVector.euler() * (180.0 / M_PI);
                float roll = eulerDegrees[0];               //x-axis
                float pitch = eulerDegrees[1];              //y-axis
                float yaw = eulerDegrees[2];                //z-axis

                debugport.printf("Roll: %.2f\n", roll);
                debugport.printf("Pitch: %.2f\n", pitch);
                debugport.printf("Yaw: %.2f\n", yaw);
                debugport.printf("\n");
         
            }
        }
    }
}
