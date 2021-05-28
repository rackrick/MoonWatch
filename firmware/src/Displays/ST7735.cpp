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

    void ST7735::showPrinterScreen(String printerName, PrinterStatus status) {
        // cleanup display on state change
        if (printerState != status.state || idleScreen) {
            tft.fillScreen(TFT_BLACK);
            printerState = status.state;
            idleScreen = false;
        }
                
        // headline        
        tft.setTextColor(TFT_WHITE, TFT_RED);  
        tft.fillRect(0, 0, 160, 13, TFT_RED);  
        tft.drawCentreString(printerName, 80, 3, 1);
        
        // temperature icons
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawBitmap(5, 16, Icons::nozzle, 16, 16, TFT_WHITE);
        tft.drawString("Extruder", 22, 19);

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawBitmap(85, 16, Icons::bed, 16, 16, TFT_WHITE);
        tft.drawString("Heatbed", 102, 19);

        // separator
        tft.drawLine(80, 15, 80, 72, TFT_DARKGREY);
        tft.drawLine(0, 75, 160, 75, TFT_DARKGREY);
        
        // extruder temperature
        tft.setTextColor(TFT_RED, TFT_BLACK);

        // two decimals for values below 100 to avoid artefacts of previous values.
        tft.fillRect(5,33, 79, 25, TFT_BLACK);        
        tft.drawCentreString(String(status.extruderTemp, 1), 40, 35, 4);        
    
        // target temperature
        tft.drawRect(5, 60, 70, 10, TFT_RED);

        // target temp == 0 display off
        if (status.extruderTarget == 0.0) {
            // draw a black Rect to clear the old bar.
            tft.fillRect(5, 60, 70, 10, TFT_BLACK);
            // redraw target bar and set to off.
            tft.drawRect(5, 60, 70, 10, TFT_RED);            
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.drawCentreString("off", 40, 61, 1);
        } else {
            tft.fillRect(
                5,                                                                      // xpos
                60,                                                                     // ypos
                calcFilledBarWidth(70, 0, status.extruderTarget, status.extruderTemp),  // width
                10,                                                                     // height
                TFT_RED                                                                 // color
            );
            tft.setTextColor(TFT_WHITE, TFT_RED);
            tft.drawCentreString(String(status.extruderTarget, 0) + " C", 40, 61, 1);                    
        }

        // bed temperature
        tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);

        // artefacts prevention
        tft.fillRect(81, 33, 79, 25, TFT_BLACK);       
        tft.drawCentreString(String(status.bedTemp, 1), 120, 35, 4);        
    
        // target temperature
        tft.drawRect(85, 60, 70, 10, TFT_SKYBLUE);

        // target temp bed == 0 display off
        if (status.bedTarget == 0.0) {
            // draw a black Rect to clear the old bar.
            tft.fillRect(85, 60, 70, 10, TFT_BLACK);
            // redraw target bar and set to off.
            tft.drawRect(85, 60, 70, 10, TFT_SKYBLUE);
            tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
            tft.drawCentreString("off", 120, 61, 1);
        } else {
            tft.fillRect(
                85,                                                             // xpos
                60,                                                             // ypos
                calcFilledBarWidth(70,0, status.bedTarget, status.bedTemp),    // width
                10,                                                             // height
                TFT_SKYBLUE                                                     // color
            ); 
            tft.setTextColor(TFT_WHITE, TFT_SKYBLUE);
            tft.drawCentreString(String(status.bedTarget, 0) + " C", 120, 61, 1);
        }

        // display progress bar on state printing
        if (status.state == "printing") {
            // progress
            if (status.progress >= 0.6) {
                tft.setTextColor(TFT_BLACK, TFT_WHITE);
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
            }            
            tft.drawRect(5, 80, 150, 20, TFT_WHITE);                
            tft.fillRect(
                        5,                                                  // xpos
                80,                                                         // ypos
                calcFilledBarWidth(150, 0, 100, status.progress * 100),     // width
                20,                                                         // height
                TFT_WHITE                                                   // color
                ); 
            tft.drawCentreString(String(status.progress * 100, 0) + " %", 80, 82, 2);

            // printing times
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.drawString("PrT:" + Toolbox::beautifyDuration(status.printDuration), 5, 105, 1);
            tft.drawString("ETA:" + Toolbox::beautifyDuration((status.printDuration / status.progress) - (float)status.printDuration), 85, 105, 1);
        } 
        // on other state we display just the state
        // TODO: beautify States
        else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.drawCentreString(status.state, 80, 82, 2);
        }

        // footer line
        tft.drawLine(0, 116, 160, 116, TFT_DARKGREY);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);     

        // damn stupid clock....
        Tools::NtpClock& internetClock = Tools::NtpClock::getInstance();       
        tft.drawCentreString(internetClock.getDateTimeString(), 80, 119, 1);
    }

    void ST7735::showIdleScreen() {                
        if (!idleScreen) {
            tft.fillScreen(TFT_BLACK);
            idleScreen = true;
            delay(1000);
        }
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        Tools::NtpClock& internetClock = Tools::NtpClock::getInstance();
        tft.drawString(internetClock.getTimeString(), 10, 32, 7);
    }

    void ST7735::showStatusMsg(String msg[], int msgCount) {                      
        tft.fillScreen(TFT_BLACK);
       
        for (byte i = 0; i < msgCount; i++) {                        
            tft.drawCentreString(msg[i], 80, i * 15, 1);            
        }
    }

    // calculates the filling of the displayed bars.    
    // valueMin, valueMax are the value limits to represent in the bar.
    int ST7735::calcFilledBarWidth(int width, int valueMin, int valueMax, float value) {

         /*         
         * scale = width / (valueMax - valueMin)
         * barSize = (value - valueMin) * scale
         */        

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