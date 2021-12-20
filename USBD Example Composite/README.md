# ft9xx-usbd-examples / USBD Example Composite
In this example, a composite device is demonstrated. 
The device will present up-to 3 interfaces and show how data can be sent or received by each interface within the framework of the FT9xx USBD driver. The USBD driver is included  in the hardware library and allows access to the USB device hardware.

## Composite Device
There are three interfaces in the example code. These will be individually controlled by the host software, it will load separate drivers for each of these interfaces. 
* Keyboard interface. 
* MIDI interface.
* DFU interface.

## Configuration
### DFU Interface
It's function is to allow a firmware update to be provisioned _via_ USB. DFU mode is enabled by sending instructions to the DFU interface. After the instructions are sent then the device will switch into "DFU mode" and provide a different interface to the host. The code to process standard requests demonstrates switching between normal mode (called "Runtime mode") and DFU mode.
The DFU interface can be disabled by setting the DISABLE_DFU macro in the source code.
To access the DFU interface from Windows it is required to a
### Keyboard Interface
For the keyboard interface the device provides an interrupt IN endpoint and report descriptors. This is in addition to the standard requests normally provided by a USB device.
As an option, controlled by the KBD_DEVICE_ENABLE macro, the code will send a series of keypresses to the host. This is disabled by default as these keypresses will be interpreted and actioned on the host because the keyboard device looks exactly like a real keyboard. 
### MIDI Interface
The MIDI interface presents as a bi-directional MIDI device to the host. It has both bulk IN and OUT endpoints. The host may send MIDI codes to the device, these will be reported on the UART interface on the device. The device will send, under the control of the MIDI_DEVICE_ENABLE macro, MIDI codes to the host. A suitable MIDI program on the host will allow the tune to be played by the host. 

## Documentation
Host applications which can be used are:
* [Virtual MIDI Piano Keyboard](https://vmpk.sourceforge.io/)
* Piano Time (Microsoft Store)
