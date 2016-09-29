================================================================================
                        SmartFusion2 Bootloader
================================================================================
The SmartFusion Bootloader is a flexible Bootloader designed to allow execution
of code in external memory (usually DDR) by loading an application image from
an SPI FLASH device into external memory and executiong it.

The SmartFusion2 Bootloader also supports maintaining and updating up to 3 
application images including a Golden or Factory Default image for recovery in
the case of an update failure.

--------------------------------------------------------------------------------
                            How to use the Bootloader
--------------------------------------------------------------------------------
For the SmartFusion2 boards from Microsemi, the following functionality of the
Bootloader is controlled via the DIP switches:

SW1 - Selects update vs execute mode. Normal operation is to execute the current
      live Firmware image.
SW2 - Selects execute Golden mode. Execute the Golden firmware image instead of
      the current live firmware image.
SW3 - Selects copy Golden mode. Copy the Golden image to the first normal
      firmware image and execute it.
      
On the Eval Kit, Security Eval Kit and Dev Kit the switch layout is:

  +--------+
  | 1 [ #] |   [# ] == selection active.   
  | 2 [ #] |   [ #] == selection inactive.   
  | 3 [ #] |      
  | 4 [ #] |      
  +--------+

On the Advanced Dev Kit the switch layout is:

  +--------+
  | [# ] 8 |   [ #] == selection active.   
  | [# ] 7 |   [# ] == selection inactive.   
  | [# ] 6 |      
  | [# ] 5 |      
  | [# ] 4 |      
  | [# ] 3 |      
  | [# ] 2 |      
  | [# ] 1 |      
  +--------+

When Update mode is selected, the Bootloader expects an Intel Hex file to be
transferred via the Ymodem protocol. Depending on configuration settings, the
Bootloader may send a Banner message via the serial port before waiting for the
file. When the Bootloader is ready to accept the file it transmits 'C' 
characters every few seconds to indicate it is in Ymodem download mode.

The hexfiles folder contains sample applications suitable for the Eval Kit and
the Advanced Dev Kit.
--------------------------------------------------------------------------------
                               UART configuration
--------------------------------------------------------------------------------
To use the YMODEM firmware update functionality of the Bootloader requires
MMUART1 of the Eval Kit/Security Eval Kit or MMUART 0 of the Dev Kit/Advanced
Dev Kit to be connected to a host PC. The host PC must connect to the serial
port using a terminal emulator such as Tera Term or ExtraPuTTY which supports
YMODEM, configured as follows:
    - 57600 baud
    - 8 data bits
    - 1 stop bit
    - no parity
    - no flow control

Connect the board and the PC using the USB MINI B connection.

For the SmartFusion2 Eval Kit/Security Eval Kit and Dev Kit the serial port to
use will be 4th serial port provided by the 4232H device and may be labelled
"USB Serial Converter D".

For the Advanced Dev Kit the serial port to use will be 3rd serial port provided
by the 4232H device and may be labelled "USB Serial Converter C".
--------------------------------------------------------------------------------
                                Target hardware
--------------------------------------------------------------------------------
The example designs for the Bootloader are built for a design using an APB clock
frequency of 83MHz (71MHz for the Security Eval Kit). Trying to execute this
example project on a different design will result in incorrect baud rate being
used by the MMUARTs or no output if the MMUARTs are not enabled and connected.
