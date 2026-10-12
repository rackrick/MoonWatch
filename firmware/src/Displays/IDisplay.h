#pragma once
#include <Arduino.h>
#include "PrinterStatus.h"

namespace RP {
    class IDisplay {
        public:
            virtual ~IDisplay() {}
            
            virtual void showPrinterScreen(const String& printerName, const PrinterStatus& status) = 0;
            virtual void showIdleScreen() = 0;
            virtual void showStatusMsg(String msg[], int msgCount) = 0;
    };
}
