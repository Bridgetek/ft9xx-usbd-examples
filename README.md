# ft9xx-usbd-examples
This repository stores sample code for the Bridgetek FT9xx series which implement emulations of USB devices. 

## Background
The FT9xx has a hardware library which is used to access to the built-in USB device hardware. The library is installed with the [FT9XX Toolchain](http://brtchip.com/ft9xx-toolchain/). Source code for the library and general purpose example code are included with the toolchain and but these can also be accessed via GitHub (Bridgetek/ft90x-sdk)[https://github.com/Bridgetek/ft90x-sdk].

Source code for other USB device examples are available as Application Notes from the Bridgetek website.

## Contents
The repository contains sample code for the following:
* USBD Example Composite - Creates a device with a Keyboard interface, MIDI interface and optional DFU interface. The keyboard and MIDI interfaces can be set to send pre-programmed data to the host to cause typing to appear on the host and play a tune to a MIDI controlled program.
* AN_414 FT90x_USBD_UVC_Webcam - Creates a USB device emulating a UVC webcam using the built-in camera interface of the FT90x and OV965x or OV5640 sensor. The FT90x appears as a Camera device and streams compressed or uncompressed data to the host PC. Compressed streams are only supported if the sensor supports a compressed output. See (Application Note AN_414)[https://brtchip.com/wp-content/uploads/sites/3/2021/07/AN-414-FT90x-UVC-WebCam.pdf]
* BRT_AN_018 FT90x Camera to EVE - Displays the output of the  built-in camera interface of the FT90x and OV5640 sensor on an EVE module. See (Application Note BRT_AN_018)[https://brtchip.com/wp-content/uploads/sites/3/2021/07/BRT_AN_018-FT90x-Camera-to-EVE.pdf]

## Documents
The following documents are available from the [BRTChip](https://brtchip.com) website: 
* [FT90X User Manual](https://brtchip.com/wp-content/uploads/Support/Documentation/Application_Notes/ICs/MCU/BRT_AN_020_FT90x_Revision_C_User_Manual.pdf). This manual provides details about the peripherals of the FT90X Revision C as well as the general system registers.
* [Toolchain Installation Guide](https://brtchip.com/wp-content/uploads/Support/Documentation/Installation_Guides/ICs/MCU/AN-325-FT9xx-Tool-Chain-Installation-Guide.pdf). Installation guide for the gcc compiler, Eclipse plugins, hardware libraries and examples for the FT9xx series.
* [Hardware API Programmers Manual](https://brtchip.com/wp-content/uploads/Support/Documentation/Programming_Guides/ICs/MCU/AN_365-FT9xx-API-Programmers-Manual.pdf). Documentation for the Hardware API for FT9xx series. This describes the API with hardware-specific libraries for the FT90x and FT93x to facilitate accessing peripherals. Prebuilt libraries and source code are installed with the toolchain.
* [Assembly Language Programming](https://brtchip.com/wp-content/uploads/Support/Documentation/Programming_Guides/ICs/MCU/AN_342_FT90X_Assembly_Language_Programming_Guide.pdf). A guide for writing code fragments in assembler for the FT90x.
Other documents are linked to in the [FT90X Produce Page](https://brtchip.com/ft900/) on the Bridgetek website.
## Examples
Example code for the hardware API is included in the toolchain installation.
* [Example Applications](https://brtchip.com/wp-content/uploads/Support/Documentation/Application_Notes/ICs/MCU/AN-360-FT9xx-Example-Applications.pdf). This guide describes each of the example codes.
* Further example code can be found on the [FT90x Software Examples](https://brtchip.com/softwareexamples-ft90x/)
