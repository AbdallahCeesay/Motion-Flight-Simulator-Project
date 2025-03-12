#include <mbed.h>
#include <BNO080.h>
#include <SerialStream.h>

#define i2cadd 0x4a
#define i2cportspeed 100000

Serial debugport(USBTX, USBRX);

PinName SDA = PB_9;
PinName SCL = PB_8;
PinName INTPin = PA_6;
PinName RSTPin = PA_5;

BNO080I2C imu(&debugport, SDA, SCL, INTPin, RSTPin, i2cadd, i2cportspeed);


int main()
{
    //BNO080I2C imu(&debugport, SDA, SCL, INTPin, RSTPin, i2cadd, i2cportspeed);
//    debugport.write("Hello World",11);

}
