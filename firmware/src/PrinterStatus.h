#include <Arduino.h>
namespace RP 
{
    struct PrinterStatus {
        bool isActive;
        float progress;
        String state;
        String filename;   
        float extruderTemp;
        float extruderTarget;
        float bedTemp;
        float bedTarget;
        String connectionState;
        int printDuration;
    };
}

