#pragma once

// configs
#include <configs/OPERATIONS_config.h>
#include <configs/LOGGING_config.h>
#include <configs/FILE_SYSTEM_config.h>
#include <configs/HARDWARE_config.h>

// bricks
#include <StatusLogger.h>

// libs
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <EEPROM.h> // read and write from flash memory
#include <LoopbackStream.h>

// Brick for the FileHandler. We will only ever use one file handler, so this is a namespace (treat as an object), not a class.
namespace FileHandler
{
    String createSessionFileName();

    File opened_file;
    LoopbackStream SESSION_Loopback(8192);

    namespace
    {
        long int pre_append_size = 0;
    }
    Stream *DEBUG_STREAM = &Serial;
    bool DEBUG_BYPASS_SD_CARD = false;
    bool SUPPRESS_BORING_MESSAGES = false;

    //*****************************************************************************
    SPIClass SPI_SD(HSPI);

    /**
     * @brief Initialize the SD Card and check whether or not we were successful
     *
     * @returns true if initialization and tests were successful, otherwise false
     */
    bool initSDCard()
    {
        if (SD.cardSize() != 0)
        {
            StatusLogger::log(StatusLogger::LEVEL_VERBOSE, StatusLogger::NAME_SD, "Already opened..");
            return true; // We've already opened the SD card
        }
        SPI_SD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS);
        for (int attempts = 0; attempts < 2; attempts++)
        // We'll try twice to make sure we're not being planks.
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            if (!SD.begin(SD_SS, SPI_SD, SPI_SPEED))
            {
                if (attempts)
                {
                    StatusLogger::setBrickStatus(StatusLogger::NAME_SD, StatusLogger::FUNCTIONALITY_OFFLINE, "Card Mount Failed");
                }
                else
                {
                    StatusLogger::setBrickStatus(StatusLogger::NAME_SD, StatusLogger::FUNCTIONALITY_OFFLINE, "Card Mount Failed. Trying again");
                }
                continue;
            }

            int cardSize = SD.cardSize() / (1024 * 1024);
            StatusLogger::setBrickStatus(StatusLogger::NAME_SD, StatusLogger::FUNCTIONALITY_FULL, "SD Card ready. Size: " + String(cardSize) + "MB");
            return true;
        }
        return false; // Fail to open SD card.
    }

    /**
     * @brief Create a new file name for the session, combining the device ID/name and the number of existing files
     *
     * @returns the file name to use
     */
    String createSessionFileName()
    {
        // EEPROM.begin(EEPROM_SIZE);
        // if (EEPROM.read(0) > MAX_FILES_ALLOWED)
        // {
        //     EEPROM.write(0, 0);
        //     EEPROM.commit();
        // }
        // int fileNumber = EEPROM.read(0);
        // EEPROM.write(0, fileNumber + 1);
        // EEPROM.commit();
        String file_name = "/" + String(DEVICE_ID) + "_" + String(now()) + ".txt";
        StatusLogger::log(StatusLogger::LEVEL_VERBOSE, StatusLogger::NAME_SD, String("Making new file: ") + file_name);
        return (file_name);
    }

    /**
     * @brief Write some message to a file at a given path
     *
     * @param fs The default filesystem (normally SD)
     * @param path The path of the file to write to
     * @param message The message you want to write to the file.
     * @returns 0 if successfully written, otherwise false
     */
    int writeFile(fs::FS &fs, const char *path, const char *message)
    {
        StatusLogger::log(StatusLogger::LEVEL_VERBOSE, StatusLogger::NAME_SD, String("Writing file: ") + path);

        File file = fs.open(path, FILE_WRITE);
        if (!file)
        {
            StatusLogger::log(StatusLogger::LEVEL_WARNING, StatusLogger::NAME_SD, "Failed to open file for writing");
            return 1;
        }
        if (file.print(message) || strcmp(message, "") == 0)
        {
            StatusLogger::log(StatusLogger::LEVEL_VERBOSE, StatusLogger::NAME_SD, "File written");
        }
        else
        {
            if (!SUPPRESS_BORING_MESSAGES)
            {
                StatusLogger::log(StatusLogger::LEVEL_WARNING, StatusLogger::NAME_SD, "Writing" + String(message) + "to SD failed. We're now suppressing this message so it doesn't clog up your console.");
                SUPPRESS_BORING_MESSAGES = true;
            }
        }
        file.close();
        return 0; // TODO: Confusing to have some functions return 0 if good, and others returning True if good
    }

    /**
     * @brief Open up a file for batch appending. Remember to close it after batch appending!
     *
     * @param fs The default filesystem (normally SD)
     * @param path The path of the file to open for appending
     * @returns true if successful and found, otherwise false
     */
    bool openFileForAppend(const char *path, fs::FS &fs = SD)
    {
        opened_file = fs.open(path, FILE_APPEND);
        if (!opened_file)
        {
            StatusLogger::log(StatusLogger::LEVEL_WARNING, StatusLogger::NAME_SD, "Failed to open file for writing");
            return false;
        }
        pre_append_size = opened_file.position();
        if (SESSION_FILE_NAME.compareTo(path) == 0)
        {
            int amount = SESSION_Loopback.available();
            while (SESSION_Loopback.available())
            {
                uint8_t buf[512];
                size_t n = min(SESSION_Loopback.available(), 512);
                for (int i = 0; i < n; i++)
                {
                    buf[i] = SESSION_Loopback.read();
                }
                opened_file.write(buf, n);
            }
        }
        return true; // TODO: Matt, put in logic here
    }

    /**
     * @brief Dump the session loopback to the Serial console.
     *
     * @return int The number of characters dumped.
     */
    int dumpSessionLoopback()
    {
        int before = SESSION_Loopback.available();

        while (SESSION_Loopback.available())
        {
            Serial.write(SESSION_Loopback.read());
        }

        return SESSION_Loopback.available() - before;
    }

    /**
     * @brief Return the contents of the Session Loopback as a string
     *
     * @return String, the contents as an Arduino string.
     */
    String stringifySessionLoopback()
    {
        String return_string = "";
        while (SESSION_Loopback.contains(EOL))
        {
            return_string += SESSION_Loopback.readStringUntil(EOL);
        }
        SESSION_Loopback.print(EOL);
        while (SESSION_Loopback.available())
        {
            return_string += char(SESSION_Loopback.read());
        }
        return return_string;
    }

    /**
     * @brief Close the file up after you've appended fully to it
     *
     * @returns The number of bytes written to the file in this time.
     */
    long int closeFileForAppend()
    {
        long int bytes_difference = opened_file.position() - pre_append_size;
        opened_file.close();
        pre_append_size = 0;
        return bytes_difference;
    }

    /**
     * @brief Append a message to a file on the SD card (can be used for any file type)
     *
     * @param fs The default filesystem (normally SD)
     * @param path The path of the file to write to
     * @param message THe message you want to append to the end of the file
     * @returns 0 if successfully appended, otherwise 1
     */
    int appendFile(fs::FS &fs, const char *path, const char *message)
    {
        unsigned long time_now = millis();
        bool close_up = false;
        if (DEBUG_BYPASS_SD_CARD && String(SESSION_HISTORY_FILENAME).compareTo(path) != 0)
        // Only used in debugging.
        {
            DEBUG_STREAM->print(message);
            return 0;
        }
        if (!opened_file)
        // No file pre-opened for append is blank, so will be opening up file for you
        {
            opened_file = fs.open(path, FILE_APPEND);
            if (!opened_file)
            {
                StatusLogger::log(StatusLogger::LEVEL_WARNING, StatusLogger::NAME_SD, "Failed to open file for writing");
                return 1;
            }
            close_up = true;
        }
        else if (strcmp(opened_file.path(), path) != 0)
        // You're trying to append to a file, but we've pre-opened a file for append (which is bad)
        {
            StatusLogger::log(StatusLogger::LEVEL_ERROR, StatusLogger::NAME_SD, "You've tried to append to a file, but you have append locked to another file!");
            return 1;
        }
        StatusLogger::log(StatusLogger::LEVEL_INTRUSIVELY_VERBOSE, StatusLogger::NAME_SD, String("Appending to file: ") + path);
        if (!opened_file)
        {
            StatusLogger::log(StatusLogger::LEVEL_WARNING, StatusLogger::NAME_SD, "Failed to open file for appending");
            return 1;
        }
        if (opened_file.print(message))
        {
            StatusLogger::log(StatusLogger::LEVEL_INTRUSIVELY_VERBOSE, StatusLogger::NAME_SD, "Message appended");
        }
        else if (strlen(message) > 0)
        {
            if (!SUPPRESS_BORING_MESSAGES)
            {
                StatusLogger::log(StatusLogger::LEVEL_WARNING, StatusLogger::NAME_SD, "Appending: " + String(message) + " to the file failed. Suppressing these messages so we don't clog up your console.");
            }

            opened_file.close();
            return 1;
        }
        if (close_up)
        {
            opened_file.close();
        }
        return 0;
    }

    /**
     * @brief Create a new file in the SD card (can be used for any file type)
     *
     * @param path The path of the file to create
     * @returns 0 if successfully created file, otherwise 1
     */
    int createFile(const char *path)
    {
        if (!SD.exists(path))
        {
            StatusLogger::log(StatusLogger::LEVEL_VERBOSE, StatusLogger::NAME_SD, "File doesn't exist. Creating file...");
            File file = SD.open(path, FILE_WRITE, true);
            file.close();
            if (writeFile(SD, path, ""))
            {
                StatusLogger::log(StatusLogger::LEVEL_ERROR, StatusLogger::NAME_SD, "Creating file on SD card failed; " + String(path));
                return 1;
            }
        }
        else
        {
            StatusLogger::log(StatusLogger::LEVEL_WARNING, StatusLogger::NAME_SD, "File already exists");
        }
        return 0;
    }

    /**
     * @brief Write a message to the Session File.
     *
     * You can pre-open the Session File for faster appending
     *
     * @param dataMessage
     */
    void writeMessageToSessionFile(String dataMessage)
    {
#ifdef VERBOSE_NMEA
        Serial.print(dataMessage);
#endif
        if (DEBUG_BYPASS_SD_CARD)
        {
            DEBUG_STREAM->print(dataMessage);
        }
        else
        {
            appendFile(SD, SESSION_FILE_NAME.c_str(), dataMessage.c_str());
        }
    }

    // Print the N2k enum type to the SD card
    template <typename T>
    void PrintN2kEnumTypeToSDnSerial(T a, fs::FS *sd, const char *filename, Stream *OutputStream, bool addLF = false)
    {
        const char *str = N2kEnumTypeToStr(a);
        if (str[0] != '\0')
        {
            if (addLF)
            {
                writeMessageToSessionFile(String(str) + EOL);
            }
            else
            {
                writeMessageToSessionFile(String(str));
            }
        }
        else
        {
            writeMessageToSessionFile("unknown (" + String(a) + ")");
        }
    }

    // Print the label and the value to the SD file inline.
    template <typename T>
    void PrintLabelValWithConversionCheckUnDef(const char *label, T val, double (*ConvFunc)(double val) = 0, bool AddLf = false, int8_t Desim = -1)
    {
        if (label != NULL)
        {
            String msg = ", ";
            msg += label;
            msg += "=";
            writeMessageToSessionFile(msg);
        }

        if (!N2kIsNA(val))
        {
            if (Desim < 0)
            {
                if (ConvFunc)
                {
#ifdef VERBOSE_NMEA
                    OutputStream->print(ConvFunc(val));
#endif
                    appendFile(SD, SESSION_FILE_NAME.c_str(), String(ConvFunc(val)).c_str());
                }
                else
                {
#ifdef VERBOSE_NMEA
                    OutputStream->print(val);
#endif
                    appendFile(SD, SESSION_FILE_NAME.c_str(), String(val).c_str());
                }
            }
            else
            {
                if (ConvFunc)
                {
#ifdef VERBOSE_NMEA
                    OutputStream->print(ConvFunc(val), Desim);
#endif
                    appendFile(SD, SESSION_FILE_NAME.c_str(), String(ConvFunc(val), Desim).c_str()); // FIXME: use desim
                }
                else
                {
#ifdef VERBOSE_NMEA
                    OutputStream->print(val, Desim);
#endif
                    appendFile(SD, SESSION_FILE_NAME.c_str(), String(val, Desim).c_str());
                }
            }
        }
        else
        {
            writeMessageToSessionFile("NA");
        }
        if (AddLf)
        {
            writeMessageToSessionFile(String(EOL));
        }
    }
}