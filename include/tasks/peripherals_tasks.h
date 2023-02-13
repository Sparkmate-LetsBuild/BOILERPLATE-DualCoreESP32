// configs
#include <configs/OPERATIONS_config.h>

// bricks
#include <bricks/file_handler.h>

// libs
#include <Arduino.h>
#include <StatusLogger.h>

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
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = 250; // happens every 250ms
        xLastWakeTime = xTaskGetTickCount();
        int count = 0;
        TickType_t ticks_before = xTaskGetTickCount();
        int spare_tick_count = 0;
        for (;;)
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
                Serial.print("Shit. Writing to the SD card took ");
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