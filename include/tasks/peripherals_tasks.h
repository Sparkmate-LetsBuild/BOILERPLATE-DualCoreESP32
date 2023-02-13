// configs
#include <configs/OPERATIONS_config.h>

// bricks
#include <bricks/file_handler.h>

// libs
#include <Arduino.h>
#include <StatusLogger.h>

// We adopt separate files and namespaces for different tasks for
// .. when our systems have a large number of tasks to jump between.
namespace PeripheralsTasks
{
    /**
     * @brief Periodically read from the Upload Buffer and write to the SD Card
     *
     * @param param
     */
    void c0_bufferWriteToSD(void *param)
    {
        unsigned long vT_t0_loop;

        // We're going to to use "ticks" to ensure we're executing a task every X milliseconds.
        // You can also use a bit of maths + elapsed time twinned with vTaskDelay(...), but this is less precise.
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = 250; // happens every 250ms
        xLastWakeTime = xTaskGetTickCount();

        // Every 1 second I want the Serial Monitor to log that we're still performing as expected
        int count = 0;
        int spare_tick_count = 0;
        TickType_t ticks_before = xTaskGetTickCount();

        for (;;)
        // run forever
        {
            vT_t0_loop = millis();

            // Open the file for appending, and stream your N2k messages
            FileHandler::openFileForAppend(SESSION_FILE_NAME.c_str());
            vTaskDelay(10); // just to give the SD card a slight breather.
            // Close the file, which returns the number of bytes written
            FileHandler::closeFileForAppend();
            int delay_for = xFrequency - (millis() - vT_t0_loop);
            if (delay_for < 0)
            {
                Serial.print("Ahhh noooo. Writing to the SD card took ");
                Serial.print(xFrequency - delay_for);
                Serial.println(" ms!");
            }
            ticks_before = xTaskGetTickCount();
            xTaskDelayUntil(&xLastWakeTime, xFrequency);
            spare_tick_count += xTaskGetTickCount() - ticks_before;
            count += 1;
            if (count >= (1000 / xFrequency))
            {
                StatusLogger::log(StatusLogger::LEVEL_GOOD_NEWS, StatusLogger::NAME_ESP32, "1s timer passed. Total ticks free: " + String(spare_tick_count));
                count = 0;
                spare_tick_count = 0;
            }
        }
    }
}