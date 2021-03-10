#ifndef IDisplayH
    #define IDisplayH
    #include "IDisplay.h"
#endif
#include "ST7735.h"

namespace RP {

    enum eDisplayMode {
        SPI_ST7735,
        I2C_HD44780_1602 
    };

    class DisplayFactory {
        public:
            static IDisplay* getInstance (eDisplayMode mode);
    };
}