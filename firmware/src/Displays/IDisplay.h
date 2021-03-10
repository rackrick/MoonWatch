#ifndef PrinterStatusH
    #define PrinterStatusH
    #include "PrinterStatus.h"
#endif
#include <Arduino.h>


namespace RP {
    class IDisplay {
        public:
            virtual ~IDisplay() {}
            
            virtual void showPrinterScreen(String printerName, PrinterStatus status) = 0;
            virtual void showIdleScreen() = 0;
            virtual void showStatusMsg(String  msg[], int msgCount) = 0;
    };
}

