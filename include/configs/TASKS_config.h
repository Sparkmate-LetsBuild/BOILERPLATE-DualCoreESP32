#pragma once

namespace TaskHandlers
{
    TaskHandle_t
        // List of all the tasks we could be running..
        // .. (use these handles to control tasks later)
        readFromAccelToBuffer,
        readFromBufferToSDCard;
}