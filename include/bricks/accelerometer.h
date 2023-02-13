#pragma once

// configs
#include <configs/OPERATIONS_config.h>

// bricks
#include <bricks/file_handler.h>
#include <bricks/time_helpers.h>

// libs
#include <Adafruit_LSM6DS3TRC.h>
#include <StatusLogger.h>

namespace Accelerometer
{
    // Adafruit_LSM6DS33 lsm6ds; -> if you have the lsm6ds33 sensor you can download it from here: https://github.com/adafruit/Adafruit_LSM6DS
    Adafruit_LSM6DS3TRC accel_chip;
    String return_accel;

    /**
     * @brief Initialise the accelerometer and run a quick check
     *
     * @returns true if initilized correctly, false if not
     */
    bool initAccel()
    {
        if (!accel_chip.begin_I2C())
        {
            StatusLogger::log(StatusLogger::LEVEL_ERROR, StatusLogger::NAME_ACCEL, "Failed to find LSM6DSR chip");
            StatusLogger::setBrickStatus(StatusLogger::NAME_ACCEL, StatusLogger::FUNCTIONALITY_OFFLINE, "Offline");
            return false;
        }
        Wire.setClock(400000U);
        accel_chip.setAccelRange(LSM6DS_ACCEL_RANGE_16_G);
        accel_chip.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);
        accel_chip.setAccelDataRate(LSM6DS_RATE_6_66K_HZ);
        accel_chip.setGyroDataRate(LSM6DS_RATE_6_66K_HZ);
        StatusLogger::setBrickStatus(StatusLogger::NAME_ACCEL, StatusLogger::FUNCTIONALITY_FULL, "Accelerometer enabled.");

        accel_chip.configInt1(false, false, true); // accelerometer DRDY on INT1
        accel_chip.configInt2(false, true, false); // gyro DRDY on INT2
        return_accel.reserve(100);                 // reserve 100 chars for this String, just helps with memory management
        return true;
    }

    /**
     * @brief Get the Accelerations as a string (calls the accelerometer straight away)
     *
     * @return String
     */
    String getAccelString(bool log_as_ms_timestamp = false, bool compressed = false)
    {

        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        accel_chip.getEvent(&accel, &gyro, &temp);
        if (!compressed)
        {
            if (log_as_ms_timestamp)
            {
                // Serial.println(String(millis()) + " : " + String(TimeHandler::now_ms()));
                return_accel = TimeHelpers::now_ms() + ",Accelerometer"; // Print the title
            }
            else
            {
                return_accel = String(now()) + ",Accelerometer"; // Print the title
            }
            return_accel += ", x=" + String(accel.acceleration.x) +
                            ", y=" + String(accel.acceleration.y) +
                            ", z=" + String(accel.acceleration.z) +
                            ", x_g=" + String(gyro.gyro.x) +
                            ", y_g=" + String(gyro.gyro.y) +
                            ", z_g=" + String(gyro.gyro.z);
        }
        else
        {
            if (log_as_ms_timestamp)
            {
                return_accel = String(millis());
            }
            else
            {
                return_accel = String(floor(millis() / 1000));
            }
            return_accel += ":" + String(accel.acceleration.x) +
                            "," + String(accel.acceleration.y) +
                            "," + String(accel.acceleration.z) +
                            ":" + String(gyro.gyro.x) +
                            "," + String(gyro.gyro.y) +
                            "," + String(gyro.gyro.z);
        }
        return return_accel;
    }
};
