#include "ST7735.h"

namespace RP {

    TFT_eSPI tft = TFT_eSPI(); 

    ST7735::ST7735()
    {
        tft.init();
        tft.setRotation(3);
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }

    // ------------------------------------------------------------------
    // printer page
    //
    // Flicker-free strategy:
    //  - the screen is wiped only when we come from another page
    //  - every value is only redrawn when it changed
    //  - text is drawn with a background colour + text padding, so old text is
    //    overwritten in one pass instead of "fill black, then draw" (= visible flash)
    // ------------------------------------------------------------------

    // static parts: icons, labels, separators
    void ST7735::drawPrinterFrame() {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawBitmap(5, 16, Icons::nozzle, 16, 16, TFT_WHITE);
        tft.drawString("Extruder", 22, 19);
        tft.drawBitmap(85, 16, Icons::bed, 16, 16, TFT_WHITE);
        tft.drawString("Heatbed", 102, 19);

        tft.drawLine(80, 15, 80, 72, TFT_DARKGREY);
        tft.drawLine(0, 75, 160, 75, TFT_DARKGREY);
        tft.drawLine(0, 116, 160, 116, TFT_DARKGREY);
    }

    void ST7735::drawTemp(int x, uint16_t color, float temp) {
        tft.setTextColor(color, TFT_BLACK);
        tft.setTextPadding(75);     // overwrites leftovers of a longer previous value
        // font 4 is 26 px high: at y=34 it ends at y=59 and stays clear of the
        // bar below (top edge at y=60). At y=35 it overwrote that edge.
        tft.drawCentreString(String(temp, 1), x + 35, 34, 4);
        tft.setTextPadding(0);
        // belt and braces: restore the top edge of the target bar
        tft.drawFastHLine(x, 60, 70, color);
    }

    void ST7735::drawTargetBar(int x, uint16_t color, float target, int fill) {
        // filled part + empty part, then the outline. No full clear -> no flash.
        if (fill > 0) {
            tft.fillRect(x, 60, fill, 10, color);
        }
        if (fill < 70) {
            tft.fillRect(x + fill, 60, 70 - fill, 10, TFT_BLACK);
        }
        tft.drawRect(x, 60, 70, 10, color);

        if (target == 0.0) {
            tft.setTextColor(color, TFT_BLACK);
            tft.drawCentreString("off", x + 35, 61, 1);
        } else {
            tft.setTextColor(TFT_WHITE, color);
            tft.drawCentreString(String(target, 0) + " C", x + 35, 61, 1);
        }
    }

    void ST7735::drawLowerArea(const PrinterStatus& status, bool printing) {
        if (printing) {
            int pct = (int)(status.progress * 100.0f + 0.5f);
            if (pct < 0) pct = 0;
            if (pct > 100) pct = 100;

            if (lowerDirty || pct != cPct) {
                int fill = calcFilledBarWidth(150, 0, 100, pct);
                if (fill > 0) {
                    tft.fillRect(5, 80, fill, 20, TFT_WHITE);
                }
                if (fill < 150) {
                    tft.fillRect(5 + fill, 80, 150 - fill, 20, TFT_BLACK);
                }
                tft.drawRect(5, 80, 150, 20, TFT_WHITE);

                if (status.progress >= 0.6) {
                    tft.setTextColor(TFT_BLACK, TFT_WHITE);
                } else {
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                }
                tft.drawCentreString(String(pct) + " %", 80, 82, 2);
                cPct = pct;
            }

            // printing times
            String prt = "PrT:" + Toolbox::beautifyDuration(status.printDuration);
            String eta;
            if (status.progress >= 0.01f) {
                int etaSecs = (int)((status.printDuration / status.progress) - (float)status.printDuration);
                if (etaSecs < 0) etaSecs = 0;
                eta = "ETA:" + Toolbox::beautifyDuration(etaSecs);
            } else {
                eta = F("ETA:--:--:--");
            }

            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextPadding(75);
            if (lowerDirty || prt != cPrT) {
                tft.drawString(prt, 5, 105, 1);
                cPrT = prt;
            }
            if (lowerDirty || eta != cEta) {
                tft.drawString(eta, 85, 105, 1);
                cEta = eta;
            }
            tft.setTextPadding(0);
        } 
        // on other state we display just the state
        // TODO: beautify States
        else {
            if (lowerDirty || status.state != cState) {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setTextPadding(150);
                tft.drawCentreString(status.state, 80, 82, 2);
                tft.setTextPadding(0);
            }
        }
        cState = status.state;
    }

    void ST7735::showPrinterScreen(const String& printerName, const PrinterStatus& status) {
        // coming from another page: wipe once and redraw everything
        if (screen != Screen::Printer) {
            tft.fillScreen(TFT_BLACK);
            screen = Screen::Printer;
            upperDirty = true;
            lowerDirty = true;
            cName = String((char)1);    // force the headline to be redrawn
            cClock = "";
            drawPrinterFrame();
        }

        // headline (also forces a full value redraw when the printer changes)
        if (printerName != cName) {
            tft.fillRect(0, 0, 160, 13, TFT_RED);
            tft.setTextColor(TFT_WHITE, TFT_RED);
            tft.drawCentreString(printerName, 80, 3, 1);
            cName = printerName;
            upperDirty = true;
            lowerDirty = true;
        }

        // switch between "printing" and "other state" layout: clear only the lower area
        bool printing = (status.state == "printing");
        if (printing != cPrinting) {
            tft.fillRect(0, 77, 160, 38, TFT_BLACK);
            cPrinting = printing;
            lowerDirty = true;
        }

        // extruder
        int extTemp = (int)(status.extruderTemp * 10.0f + 0.5f);
        int extTarget = (int)(status.extruderTarget + 0.5f);
        int extFill = (status.extruderTarget == 0.0) ? 0 : calcFilledBarWidth(70, 0, status.extruderTarget, status.extruderTemp);

        if (upperDirty || extTemp != cExtTemp) {
            drawTemp(5, TFT_RED, status.extruderTemp);
            cExtTemp = extTemp;
        }
        if (upperDirty || extTarget != cExtTarget || extFill != cExtFill) {
            drawTargetBar(5, TFT_RED, status.extruderTarget, extFill);
            cExtTarget = extTarget;
            cExtFill = extFill;
        }

        // bed
        int bedTemp = (int)(status.bedTemp * 10.0f + 0.5f);
        int bedTarget = (int)(status.bedTarget + 0.5f);
        int bedFill = (status.bedTarget == 0.0) ? 0 : calcFilledBarWidth(70, 0, status.bedTarget, status.bedTemp);

        if (upperDirty || bedTemp != cBedTemp) {
            drawTemp(85, TFT_SKYBLUE, status.bedTemp);
            cBedTemp = bedTemp;
        }
        if (upperDirty || bedTarget != cBedTarget || bedFill != cBedFill) {
            drawTargetBar(85, TFT_SKYBLUE, status.bedTarget, bedFill);
            cBedTarget = bedTarget;
            cBedFill = bedFill;
        }
        upperDirty = false;

        // progress / state
        drawLowerArea(status, printing);
        lowerDirty = false;

        // footer: clock (redraws over itself, no clearing needed)
        Tools::NtpClock& internetClock = Tools::NtpClock::getInstance();
        String clock = internetClock.getDateTimeString();
        if (clock != cClock) {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextPadding(120);
            tft.drawCentreString(clock, 80, 119, 1);
            tft.setTextPadding(0);
            cClock = clock;
        }
    }

    // ------------------------------------------------------------------
    // idle page (clock only)
    // ------------------------------------------------------------------
    void ST7735::showIdleScreen() {
        if (screen != Screen::Idle) {
            tft.fillScreen(TFT_BLACK);
            screen = Screen::Idle;
            cIdleClock = "";
        }

        Tools::NtpClock& internetClock = Tools::NtpClock::getInstance();
        String t = internetClock.getTimeString();
        if (t != cIdleClock) {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextPadding(140);
            tft.drawString(t, 10, 32, 7);
            tft.setTextPadding(0);
            cIdleClock = t;
        }
    }

    // ------------------------------------------------------------------
    // status messages (boot, wifi, errors)
    // ------------------------------------------------------------------
    void ST7735::showStatusMsg(String msg[], int msgCount) {
        String key;
        for (int i = 0; i < msgCount; i++) {
            key += msg[i];
            key += '\n';
        }
        // identical message already on screen -> nothing to do
        if (screen == Screen::Status && key == cStatusKey) {
            return;
        }

        // wipe only when coming from another page or when the line count shrinks
        if (screen != Screen::Status || msgCount < cStatusCount) {
            tft.fillScreen(TFT_BLACK);
        }
        screen = Screen::Status;

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextPadding(160);
        for (byte i = 0; i < msgCount; i++) {                        
            tft.drawCentreString(msg[i], 80, i * 15, 1);            
        }
        tft.setTextPadding(0);

        cStatusKey = key;
        cStatusCount = msgCount;
    }

    // calculates the filling of the displayed bars.    
    // valueMin, valueMax are the value limits to represent in the bar.
    int ST7735::calcFilledBarWidth(int width, int valueMin, int valueMax, float value) {

         /*         
         * scale = width / (valueMax - valueMin)
         * barSize = (value - valueMin) * scale
         */        

        if (valueMax == valueMin) {
            return 0;
        }

        float scale = width / (float)(valueMax - valueMin);        
        int result = (value - valueMin) * scale;

        // limit the bar to min and max
        if (result > width) {
            result = width;
        }

        if (result < 0) {
            result = 0;
        }

        return result;
    }
}
