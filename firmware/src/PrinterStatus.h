#pragma once
#include <Arduino.h>

namespace RP 
{
    struct PrinterStatus {
        // true when the data was fetched and parsed successfully
        bool valid = false;
        bool isActive = false;
        float progress = 0.0f;
        String state;
        String filename;   
        float extruderTemp = 0.0f;
        float extruderTarget = 0.0f;
        float bedTemp = 0.0f;
        float bedTarget = 0.0f;
        // "success" or a short error description (for serial debugging)
        String connectionState;
        int printDuration = 0;
    };
}
