// include libraries
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

// guarded includes
#ifndef IDisplayH
    #define IDisplayH
    #include "IDisplay.h"
#endif

#ifndef NtpClockH
    #define NtpClockH
    #include "tools/NtpClock.h"
#endif

// local includes
#include "../Icons/bed.h"
#include "../Icons/nozzle.h"
#include "../toolbox.h"

namespace RP
{
    class ST7735 : public IDisplay {
        public:
            ST7735();
            // void initScreen();
            void showPrinterScreen(String printerName, PrinterStatus status);
            void showIdleScreen();
            void showStatusMsg(String msg[], int msgCount);
            
        private:
            String printerState;
            bool idleScreen = false;
            
            // linear function to calculate the filling of temp and progress bars.
            int calcFilledBarWidth(                
                int width,
                int valueMin,
                int valueMax,
                float value                
            );
            

    };
} // namespace RP
