#pragma once

// Define constants
#define ACCEL_RATE 1000 // operatate the accelerometer at 1kHz

#ifndef DEVICE_ID
#define DEVICE_ID "testing" // happens if we don't set this value automatically on compile
#endif

#ifndef ENVIRONMENT
#define ENVIRONMENT "UNKNOWN"
#endif

// GLOBAL VARIABLES
String SESSION_FILE_NAME;