#include "mbed.h"
#include <BNO080.h>
#include <SerialStream.h>

#define SPI_FREQ 3000000
PinName SCK = PE_2;
PinName CS = PE_4;
PinName MISO = PE_5;
PinName MOSI = PE_6;
PinName INTPIN = PF_13;
PinName RSTPIN = PE_9;
PinName PS0 = PE_11; // SPI wakeup pin PS0 on IMU
PinName PS1 = PE_14;



int main()
{

    BufferedSerial serial(USBTX, USBRX, 9600);
    SerialStream<BufferedSerial> debugport(serial);


    BNO080SPI imu(debugport, RSTPIN, INTPIN, PS0, MISO, MOSI, SCK, CS, SPI_FREQ);
    while (true) {

    }
}

