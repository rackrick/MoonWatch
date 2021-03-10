#include <Arduino.h>
#include "MoonWatch.h"

RP::MoonWatch monitor;
void setup() {
  monitor.setupMonitor();
}

void loop() {
  monitor.startMonitor();
}
