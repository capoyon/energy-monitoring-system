# Home Energy Monitoring and Control System
This project presents an energy monitoring and control system leveraging the capabilities of an ESP32 microcontroller.

## Overview

### Hardware:
* ESP-WROOM-32 microcontroller
* SCT013 AC current sensor
* ZMPT101B AC voltage sensor

### Development: 
[Espressif IoT Development Framework](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) is use in developing this project.
#### Compilation:
    idf.py build
#### Flashing:
    idf.py flash && idf.py monitor
