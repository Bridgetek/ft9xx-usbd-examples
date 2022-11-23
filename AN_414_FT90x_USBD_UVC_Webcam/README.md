# ft9xx-usbd-examples / AN_414 FT90x USBD UVC Webcam
In this example, a device which emulates a UVC webcam is demonstrated. The device is compatible with the 
Video Device Class Specification found on the [USB-IF](https://usb.org) website.

## Composite Device
The device will present a 2 interfaces; the first is a Video Control Interface which allows the host PC 
to understand the capabilities of the device; the second is a Video Streaming Interface. 

## Documentation
The documentation for the device class is found in the [USB Video Class 1.1 Document Set](https://www.usb.org/document-library/video-class-v11-document-set).

## Testing
1. Connect the programmed device to a Windows 11 host PC. 
2. On Windows 11 open "Settings", "Bluetooth and Devices", then "Cameras". 
3. The device will appear as "AN_414 UVC ISOC" under "Connected Cameras". Click on the device and a preview will appear.
