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

// GPIO
DigitalOut rst(RSTPin);
BufferedSerial serial(USBTX, USBRX, 9600);
SerialStream<BufferedSerial> debugport(serial);
BNO080I2C imu(&debugport, SDA, SCL, INTPin, RSTPin, i2cadd, i2cportspeed);

// functions

// ISRS

int main() {

    // reset the sensor -- active low
    rst = 0;
    wait_us(100000);    // 100ms
    rst = 1;            // de-assert rst

    debugport.printf("============================================\n");

    if(imu.begin()) printf("Init Success!\n"); // check that the init succeeded
    else printf("Init Failed!\n");

    // Configure IMU reports
    imu.enableReport(BNO080::ROTATION, 10);

    while (true) {
        
        if (imu.updateData()) {

            if (imu.hasNewData(BNO080::ROTATION)) {
                
                TVector3 eulerDegrees = imu.rotationVector.euler() * (180.0 / M_PI);
                debugport.printf("Rotation (deg): ");
                eulerDegrees.print(debugport, true);
                debugport.printf("\n");
            }
        }
    }
}
