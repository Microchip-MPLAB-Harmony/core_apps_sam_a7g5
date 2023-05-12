---
parent: Harmony 3 driver and system service application examples for SAM A7G5 family
title: SPI Driver synchronous - multi client 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# SPI Driver synchronous - multi client

This example demonstrates how to use single instance of the SPI driver in synchronous mode to communicate with multiple EEPROMs in an RTOS environment.

## Description

This example writes and reads data to and from two separate EEPROMs connected over the same SPI bus by using the multi client feature of a synchronous SPI driver. The example also demonstrates how to setup two different client transfers at two different baud rates.

The example has three RTOS threads for the purpose:

- **APP_EEPROM1_Tasks**: This thread performs write-read operation on the first EEPROM
- **APP_EEPROM2_Tasks**: This thread performs write-read operation on the second EEPROM
- **APP_MONITOR_Tasks**: This thread checks the status of the EEPROM transfers and turns on the LED if the transfers are successful

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/core_apps_sam_a7g5) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/driver/spi/sync/spi_multi_slave/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_a7g5_ek_freertos.X | MPLABX project for [SAMA7G5 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV21H18A) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_a7g5_ek_freertos.X | [SAMA7G5 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV21H18A) |
|||

### Setting up [SAMA7G5 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV21H18A)

#### Addtional hardware required

- SD Card with FAT32 file system
- Two [EEPROM 4 Click](https://www.mikroe.com/eeprom-4-click) boards

#### Setting up the SD Card

- Download harmony MPU bootstrap loader from this [location](firmware/at91bootstrap_sam_a7g5_ek_freertos.X/binaries/boot.bin)
- Copy the downloaded boot loader binary( boot.bin) onto the SD card

#### Setting up the board

- Install [EEPROM 4 Click1](https://www.mikroe.com/eeprom-4-click) boards on to the MikroBus1 connector
- Install [EEPROM 4 Click2](https://www.mikroe.com/eeprom-4-click) boards on to the MikroBus2 connector
- SDMMC slot used for bootloading the application is SDMMC1 (J4)
- Connect the USB port J7 on board to the computer using a micro USB cable (to power the board)
- Connect the USB port J24 on board to the computer using a micro USB cable (to enable debug com port)

## Running the Application

1. Build the application using its IDE
2. Copy the output binary (named 'harmony.bin') onto the SD Card (Refer to the 'Setting up hardware' section above for setting up the SD card)
3. Insert the SD card into SDMMC slot on the board (Refer to the 'Setting up hardware' section for the correct SDMMC slot)
4. Reset the board to run the application
5. The LED turns ON when the data read from the EEPROMs matches with the data written to the EEPROMs.

Refer to the following table for LED name:

| Board | LED Name |
| ----- | -------- |
|  [SAMA7G5 Evaluation Kit](https://www.microchip.com/en-us/development-tool/EV21H18A)  | RGB_LED(Red) |
|||
