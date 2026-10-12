#pragma once
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
            void showPrinterScreen(const String& printerName, const PrinterStatus& status);
            void showIdleScreen();
            void showStatusMsg(String msg[], int msgCount);
            
        private:
            // which "page" is currently on the glass. The screen is only wiped
            // when this changes, never while updating values on the same page.
            enum class Screen { None, Printer, Idle, Status };
            Screen screen = Screen::None;

            // --- cached values of the printer page: only redraw what changed ---
            bool upperDirty = true;     // header, temperatures, bars
            bool lowerDirty = true;     // progress / state area
            bool cPrinting = false;
            String cName;
            String cState;
            String cPrT;
            String cEta;
            String cClock;
            int cExtTemp = 0;           // temp * 10
            int cExtTarget = 0;
            int cExtFill = 0;
            int cBedTemp = 0;
            int cBedTarget = 0;
            int cBedFill = 0;
            int cPct = 0;

            // --- cached values of the idle / status page ---
            String cIdleClock;
            String cStatusKey;
            int cStatusCount = 0;

            void drawPrinterFrame();
            void drawTemp(int x, uint16_t color, float temp);
            void drawTargetBar(int x, uint16_t color, float target, int fill);
            void drawLowerArea(const PrinterStatus& status, bool printing);

            // linear function to calculate the filling of temp and progress bars.
            int calcFilledBarWidth(                
                int width,
                int valueMin,
                int valueMax,
                float value                
            );
    };
} // namespace RP
