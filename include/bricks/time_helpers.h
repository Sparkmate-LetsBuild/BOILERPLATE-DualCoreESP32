#pragma once

// libs
#include <TimeLib.h>

namespace TimeHelpers
{
    /**
     * @brief Report the time in ms as a string
     *
     * @return String
     */
    String now_ms()
    {
        String time_in_seconds;
        String add_ms;
        time_in_seconds = String(now());
        add_ms = String(millis() % 1000);
        int len_add_ms = add_ms.length();
        if (len_add_ms == 0 or len_add_ms > 3)
        // if the length of the add_ms is 0 chars long or over 3 chars long (maths issue)
        {
            add_ms = "000";
        }
        else if (len_add_ms == 1)
        // if the length of the add_ms is 1 chars long
        {
            add_ms = "00" + add_ms;
        }
        else if (len_add_ms == 2)
        // if the length of the add_ms is 2 chars long
        {
            add_ms = "0" + add_ms;
        }
        time_in_seconds += add_ms;
        return time_in_seconds; // add the 0->999 part of the ms timer
    }
}