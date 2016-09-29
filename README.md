# RISC-V Bare Metal Boot Loader
Boot loader for bare metal applications. Loads an application from SPI flash
into DDR memory and starts application running in DDR memory.
A DIP switch allows selectiong between running the application on reset or
updating the application stored in SPI flash.
The application update is transfered over the UART port using the YModem
protocol.

This code is only targeted at the M2S150 Advanced Development Kit for now.
The associated Libero design is avaialble from https://github.com/RISCV-on-Microsemi-FPGA/M2S150-Advanced-Dev-Kit

