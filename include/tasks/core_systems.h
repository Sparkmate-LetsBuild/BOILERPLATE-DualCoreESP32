#pragma once

// configs
#include <configs/HARDWARE_config.h>
#include <configs/OPERATIONS_config.h>

// bricks
#include <inits/firmware_details_init.h>
#include <bricks/file_handler.h>
#include <bricks/accelerometer.h>

// libs
#include <StatusLogger.h>

namespace CoreSystemsTasks
{
    /**
     * @brief Initialize the core board systems (serial begins, wire libs, pin definitions)
     */
    void initBoard()
    {
        // Begin core systems
        Serial.begin(Serial_Mon_Baud);

        // Begin firmware reporting
        Firmware::init();
    }

    /**
     * @brief Initialize the core bricks (SD card and RTC clock)
     *
     */
    void initCoreBricks()
    {
        if (!FileHandler::initSDCard())
        {
            Serial.print("Failed to find the SD card. I can't start the session");
            while (1)
            {
                delay(1000);
                Serial.print(".");
            }
        }

        if (!Accelerometer::initAccel())
        {
            Serial.print("Failed to find the accelerometer. I can't start the session");
            while (1)
            {
                delay(1000);
                Serial.print(".");
            }
        }
    }

    /**
     * @brief Initialize the session and be ready to log data
     */
    bool initSession()
    {
        // Step 4: create a new session file
        SESSION_FILE_NAME = FileHandler::createSessionFileName();
        FileHandler::createFile(SESSION_FILE_NAME.c_str());
        StatusLogger::setSessionDetails(DEVICE_ID, SESSION_FILE_NAME, now());

        // Step 5. Write the session details in the upload file
        FileHandler::appendFile(SD, SESSION_FILE_NAME.c_str(), String("ID=" + String(DEVICE_ID) + ",SST=" + String(now()) + String(EOL)).c_str());

        return true;
    }
}
