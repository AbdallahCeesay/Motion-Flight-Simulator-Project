#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <iomanip> // Required for std::fixed and std::setprecision
#include "SimConnect.h"
#include <string> // Required for std::to_string

// Forward declaration for the dispatch proc
void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);

enum DATA_DEFINE_ID {
    DEFINITION_FLIGHT_DATA
};

enum DATA_REQUEST_ID {
    REQUEST_FLIGHT_DATA
};

// Data structure to receive simulation data
struct FlightData {
    double pitch;     // degrees
    double bank;      // degrees
    double heading;   // degrees
};

HANDLE hSimConnect = nullptr;
bool g_quit = false; // Flag to signal program termination

// Callback function to handle received SimConnect messages
void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
    switch (pData->dwID) {
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
        SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA*>(pData);

        // Check if this data is the one we requested
        if (pObjData->dwRequestID == REQUEST_FLIGHT_DATA) {
            // Cast the received data block to our FlightData struct
            FlightData* data = reinterpret_cast<FlightData*>(&pObjData->dwData);

            // Print the data to the console, overwriting the previous line
            std::cout << "\rPitch: " << std::fixed << std::setprecision(2) << data->pitch
                << "°,  Roll: " << std::fixed << std::setprecision(2) << data->bank
                << "°,  Yaw: " << std::fixed << std::setprecision(2) << data->heading << "°       " // Added padding spaces
                << std::flush;
        }
        break;
    }

    case SIMCONNECT_RECV_ID_QUIT: {
        // Simulator has closed the connection
        std::cerr << "\nSimConnect connection closed by simulator." << std::endl;
        g_quit = true; // Signal the main loop to exit
        break;
    }

    case SIMCONNECT_RECV_ID_EXCEPTION: {
        // An exception occurred within SimConnect
        SIMCONNECT_RECV_EXCEPTION* pEx = reinterpret_cast<SIMCONNECT_RECV_EXCEPTION*>(pData);
        std::cerr << "\nSimConnect Exception received: Code=" << pEx->dwException
            << " SendID=" << pEx->dwSendID
            << " Index=" << pEx->dwIndex << std::endl;
        // Depending on the severity, you might want to quit
        // For simplicity here, we will quit on any exception
        g_quit = true;
        break;
    }

    case SIMCONNECT_RECV_ID_OPEN: {
        // Connection successfully opened (though we already know this from SimConnect_Open)
        std::cout << "SimConnect connection opened event received." << std::endl;
        break;
    }

    case SIMCONNECT_RECV_ID_EVENT: {
        // Handle other events if needed in the future
        break;
    }

    default: {
        // Optional: Log unhandled message types
        // std::cout << "\nReceived unhandled SimConnect message ID: " << pData->dwID << std::endl;
        break;
    }
    }
}

int main() {
    HRESULT hr; // Variable to store function results

    // Attempt to open a connection to SimConnect
    hr = SimConnect_Open(&hSimConnect, "Live Telemetry Reader", nullptr, 0, 0, 0);

    if (SUCCEEDED(hr)) {
        std::cout << "Connected to MSFS via SimConnect!" << std::endl;

        // Define the data structure members we want to receive
        // IMPORTANT: The order MUST match the FlightData struct definition
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_FLIGHT_DATA, "PLANE PITCH DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
        if FAILED(hr) std::cerr << "Error adding pitch definition: " << hr << std::endl;

        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_FLIGHT_DATA, "PLANE BANK DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
        if FAILED(hr) std::cerr << "Error adding bank definition: " << hr << std::endl;

        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_FLIGHT_DATA, "PLANE HEADING DEGREES MAGNETIC", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
        if FAILED(hr) std::cerr << "Error adding heading definition: " << hr << std::endl;


        // Request the defined data structure periodically (per simulation frame)
        hr = SimConnect_RequestDataOnSimObject(
            hSimConnect,
            REQUEST_FLIGHT_DATA,              // Unique ID for this request
            DEFINITION_FLIGHT_DATA,           // ID of the data definition to use
            SIMCONNECT_OBJECT_ID_USER,        // Request data for the user's aircraft
            SIMCONNECT_PERIOD_SIM_FRAME,      // *** Request data every simulation frame (high speed) ***
            SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT, // Send data normally (consider _CHANGED if needed later)
            0,                                // No offset within the period
            0,                                // No interval limit
            0                                 // *** Corrected: dwArrayCount should be 0 for single struct ***
        );
        if FAILED(hr) std::cerr << "Error requesting data: " << hr << std::endl;


        // Main message processing loop
        std::cout << "Starting real-time data processing loop..." << std::endl;
        while (!g_quit) {
            // Process any pending SimConnect messages
            hr = SimConnect_CallDispatch(hSimConnect, MyDispatchProc, nullptr);

            if (FAILED(hr)) {
                // An error occurred during message dispatch (e.g., connection lost)
                std::cerr << "\nSimConnect_CallDispatch failed with HRESULT: 0x"
                    << std::hex << hr << std::dec << std::endl;
                g_quit = true; // Signal loop termination
                break;         // Exit the loop immediately
            }

            // Sleep briefly to avoid consuming 100% CPU if the dispatch call returns very quickly
            // without processing any messages. Adjust as needed or remove if CPU usage is acceptable.
            Sleep(1);
        }
        std::cout << std::endl << "Exiting processing loop." << std::endl; // Move to next line after loop ends


        // Clean up and close the SimConnect connection
        hr = SimConnect_Close(hSimConnect);
        if (FAILED(hr)) {
            std::cerr << "Error closing SimConnect connection: " << hr << std::endl;
        }
        else {
            std::cout << "SimConnect connection closed." << std::endl;
        }
    }
    else {
        // Failed to open the connection
        std::cerr << "Failed to connect to MSFS. HRESULT: 0x"
            << std::hex << hr << std::dec << std::endl;
        std::cerr << "Ensure MSFS is running and SimConnect is installed/configured correctly." << std::endl;
    }

    std::cout << "Program finished. Press Enter to exit." << std::endl;
    std::cin.get(); // Keep console window open until user presses Enter

    return 0;
}