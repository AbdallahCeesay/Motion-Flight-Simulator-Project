#include <mbed.h>
#include <BNO080.h>
#include <SerialStream.h>

#define i2cadd 0x4A    // Confirm I2C address with your hardware
#define i2cportspeed 100000

// Define your hardware pins (adjust for your board)
PinName SDA = PB_9;
PinName SCL = PB_8;
PinName INTPin = PA_6;
PinName RSTPin = PA_5;

int main() {

    BufferedSerial serial(USBTX, USBRX, 9600);
    SerialStream<BufferedSerial> debugport(serial);

    BNO080I2C imu(&debugport, SDA, SCL, INTPin, RSTPin, i2cadd, i2cportspeed);

    debugport.printf("============================================\n");

    imu.begin();

    // Configure IMU reports
    imu.enableReport(BNO080::ROTATION, 100);

    while (true) {
        
        ThisThread::sleep_for(1ms);

        if (imu.updateData()) {

            if (imu.hasNewData(BNO080::ROTATION)) {
                
                TVector3 eulerDegrees = imu.rotationVector.euler() * (180.0 / M_PI);
                debugport.printf("Rotation (deg): ");
                eulerDegrees.print(debugport, true);
                debugport.printf("\n");
            }
            // if (imu.hasNewData(BNO080::TOTAL_ACCELERATION)) {
            //     debugport.printf("Acceleration: ");
            //     imu.totalAcceleration.print(debugport, true);
            //     debugport.printf("\n");
            // }
        }
    }
}