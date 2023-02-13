#pragma once

// SD Card defines
#define EEPROM_SIZE 1
#define MAX_FILES_ALLOWED 256     // maximum number of file named until back from file_0

// Global definitions
#define UNIVERSAL_DELIMITER '|' // You CAN NOT use this char in file path names as we are treating it as a universal delimiter
#define EOL '\n'
#define MAX_FILES_ALLOWED_HISTORY_LINE_LENGTH 180
#define SESSION_HISTORY_FILENAME "/upload_history.txt"

// Streams
Stream *OutputStream;
#define UPLOAD_BUFFER_SIZE 32000 // we will use up to a 32kb upload buffer