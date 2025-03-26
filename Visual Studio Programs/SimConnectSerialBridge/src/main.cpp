#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include "SimConnect.h"

HANDLE hSimConnect = NULL;
HANDLE hSerial;

// Struct for holding flight data
struct FlightData {
    double pitch;
    double roll;
    double altitude;
    double airspeed;
};

enum DATA_DEFINE_ID {
    DEFINITION_1
};

enum DATA_REQUEST_ID {
    REQUEST_1
};

void setupSerial(const char* portName) {
    hSerial = CreateFileA(portName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port!" << std::endl;
        exit(1);
    }
}

void sendToSerial(const FlightData& data) {
    std::ostringstream oss;
    oss << "P:" << data.pitch << ";R:" << data.roll << ";ALT:" << data.altitude << ";SPD:" << data.airspeed << ";
";
    std::string output = oss.str();

    DWORD bytesWritten;
    WriteFile(hSerial, output.c_str(), output.length(), &bytesWritten, NULL);
}

void CALLBACK DispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
    switch (pData->dwID) {
        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
            SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
            if (pObjData->dwRequestID == REQUEST_1) {
                FlightData* data = (FlightData*)&pObjData->dwData;
                sendToSerial(*data);
                std::cout << "Sent: P:" << data->pitch << " R:" << data->roll << " ALT:" << data->altitude << " SPD:" << data->airspeed << std::endl;
            }
            break;
        }
    }
}

int main() {
    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "SimConnect Bridge", NULL, 0, 0, 0))) {
        setupSerial("COM5");  // Change COM port as needed

        SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "PLANE PITCH DEGREES", "degrees");
        SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "PLANE BANK DEGREES", "degrees");
        SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "PLANE ALTITUDE", "feet");
        SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "AIRSPEED INDICATED", "knots");

        while (true) {
            SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST_1, DEFINITION_1, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_VISUAL_FRAME, 0, 0, 0, 0);
            SimConnect_CallDispatch(hSimConnect, DispatchProc, NULL);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        SimConnect_Close(hSimConnect);
        CloseHandle(hSerial);
    } else {
        std::cerr << "Could not connect to SimConnect." << std::endl;
    }

    return 0;
}