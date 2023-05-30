---
parent: Harmony 3 driver and system service application examples for SAMA7G5 family
title: I2C EEPROM AT24 driver - EEPROM Read Write 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# I2C EEPROM AT24 driver - EEPROM Read Write

This example application shows how to use the AT24 driver to perform read and write operations on AT24 series of EEPROM.

## Description

- This example uses the AT24 driver to communicate with I2C based External AT24CM0 series EEPROMs to perform write and read operations in Bare-Metal environment.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/core_apps_sam_a7g5) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/driver/i2c_eeprom/at24/at24_eeprom_read_write/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a7g5_ek.X | MPLABX project for [SAMA7G5 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV21H18A) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a7g5_ek.X | [SAMA7G5 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV21H18A) |
|||

### Setting up [SAMA7G5 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV21H18A)

#### Addtional hardware required

- One [EEPROM 3 Click](https://www.mikroe.com/eeprom-3-click) board

#### Setting up the board

- Install the [EEPROM 3 Click](https://www.mikroe.com/eeprom-3-click) board on to the mikroBUS1 connector
- Connect the USB port J7 on board to the computer using a micro USB cable (to power the board)
- Connect the USB port J24 on board to the computer using a micro USB cable (to enable debug com port)

## Running the Application

1. Build the application using its IDE
2. LED is turned ON when the data read from EEPROM matches with the written data

Refer to the following table for LED name:

| Board | LED Name |
| ----- | -------- |
|  [SAMA7G5 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV21H18A)  | RGB_LED(Green) |
|||