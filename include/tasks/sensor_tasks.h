// configs
#include <configs/OPERATIONS_config.h>

//bricks
#include <bricks/accelerometer.h>

namespace SensorTasks
{
    /**
     * @brief Periodically write the accelerometer's values to the buffer

     */
    void c1_writeAccelToBuffer(void *param)
    {
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = ACCEL_RATE / 1000;
        for (;;)
        {
            // Sniff from Accel to Buffer
            FileHandler::SESSION_Loopback.println(Accelerometer::getAccelString(true));
            // Wait for the next cycle.
            xTaskDelayUntil(&xLastWakeTime, xFrequency);
        }
    }
}
