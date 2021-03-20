# Getting started guide

This document gives an explanation how to setup your own MoonWatch!

## Bill of Materials

- 1,8" TFT SPI Display ST7735 (Support for additonal Displays is coming soon)
- D1 Mini - ESP8266 
- Up to 5 WS2812B LEDs
- A few jumper cables and dupont connectors (see wiring)
- 4 pcs of M3x10 self tapping screws

## Printlist

- 1 Frontplate
- 1 Backplate
- 1 Enclosure
- 3 LED Clips


**Print settings:**

Nozzle Diameter: 0.4mm<br />
Layer Height: 0.2mm<br />
Print without supports<br />

## Wiring

ADD WIRING SCHEME HERE

## Software
### Flash firmware

#### Windows guide (*NIX systems follow soon)

In order to flash the software you first need to install the CH340 USB-Driver for the ESP8266.
Download and install it from [https://wemos.cc](https://www.wemos.cc/en/latest/ch340_driver.html). After installing get the [lates release](https://github.com/rackrick/MoonWatch/releases) of MoonWatch. Unzip and run `moonwatch-flash.bat`. 

**Warning: Thil will erase your ESP8266 and will overwrite it with the MoonWatch firmware.**

Enter the COM-Port of your ESP8266 and hit enter twice to start the flashing process. 
If you don't know the COM-Port number open the Windows Device Manger -> Ports (COM & LPT) and look for `USB-Serial CH340`.

Is the firmware sucessfully flashed to your ESP8266 the MoonWatch should boot now and display the setup instructions.

### Configuration

As soon the Moonwatch starts it will open the `MoonWatch` wifi network for the initial setup.
Connect with your wifi device of joice to the  MoonWatch network and browse to [http://192.168.4.1](http://192.168.4.1).
Enter your wifi network name and password and hit save. The interface should prompt you to reset your Moonwatch to apply the new settings.
After the restart it connects to your own wifi and displays the ip optained from your router. 

**Note:** The Configuration Network and the storage of the MoonWatch is not encrypted. Make sure to use MoonWatch in an safe envoirment.

On an successfull connection to your local wifi network. MoonWatch will disable the configuration hotspot and is reachable with the IP displayed on startup.
Use the Interface to add your printers to watch and enjoy your MoonWatch.


## Interface Settings

#### General
- Wifi Network: The SSID / Name of your network
- Wifi Password: Your password comes here
- Display Type: For future use to select the installed display.

#### Printers

Button `Add Printer`: Add a new printer to your configuration<br />
Button `Remove Printer`: Removes the last printer from your configuration

- Printer Name: Display name of your printer.
- Host: Hostname or IP-Adress of your printer.
- Led#: 0 based index of the avaible LEDs. This option defines which LED shows the status of this printer.

#### LED

- Number of LEDs: Sets the number of avaible LEDs on your LED stripe.
- Status Led#: 0 based index of avaible LEDs. This option defines which LED shows the MoonWatch system status.<br /> 
(0 = LED 1 on the stripe, 1 = LED 2, ...)
- Brightness: Sets the brightness of your LED stripe. 0-255 (0 = off, 255 = max power).

## Troubleshooting

1. Display stays white<br />
check your wiring.

2. LED stripe is not working<br />
the WS2812B have a direction from where the data shoule be coming. Check the arrow on the stripe.



