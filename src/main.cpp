// include Arduino.h first to avoid squiggles
#include <Arduino.h>

// configs
#include <configs/HARDWARE_config.h>
#include <configs/OPERATIONS_config.h>
#include <configs/TASKS_config.h>

// task handler
#include <configs/TASKS_config.h>

// tasks
#include <tasks/core_systems.h>
#include <tasks/sensor_tasks.h>
#include <tasks/peripherals_tasks.h>

void setup()
{
    // Setup the board for normal operations
    CoreSystemsTasks::initBoard();      // no need for it to be a task, as we need it to block both cores anyway
    CoreSystemsTasks::initCoreBricks(); // no need for it to be a task, as we need it to block both cores anyway
    CoreSystemsTasks::initSession();    // no need for it to be a task, as we need it to block both cores anyway

    // CORE 0 - Use core 0 to write from the buffer to the SD card
    xTaskCreatePinnedToCore(                   //
        PeripheralsTasks::c0_bufferWriteToSD,  // Function that should be called
        "Write to SD",                         // Name of the task (for debugging)
        4096,                                  // Stack size (bytes)
        NULL,                                  // Parameter to pass
        5,                                     // Task priority
        &TaskHandlers::readFromBufferToSDCard, // Task handle
        0                                      // Pin to core 0
    );

    // CORE 1 - Use core 1 to get the Accelerometer values and write to the buffer
    xTaskCreatePinnedToCore(                  //
        SensorTasks::c1_writeAccelToBuffer,   // Function that should be called
        "Write to SD",                        // Name of the task (for debugging)
        4096,                                 // Stack size (bytes)
        NULL,                                 // Parameter to pass
        5,                                    // Task priority
        &TaskHandlers::readFromAccelToBuffer, // Task handle
        0                                     // Pin to core 0
    );
}

// We won't be using the loop today (but it's essentially a forever running loop on Core 1)
void loop()
{
    vTaskDelete(NULL);
}
