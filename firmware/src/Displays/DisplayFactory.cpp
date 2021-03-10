#include "DisplayFactory.h"

namespace RP {
    IDisplay* DisplayFactory::getInstance (eDisplayMode mode) {

        IDisplay* pDisplay;

        switch (mode) {
            case eDisplayMode::SPI_ST7735:
                pDisplay = new ST7735();
                break;

            default:
                pDisplay = new ST7735();     
                break;       
        }

        return pDisplay;
    }
}