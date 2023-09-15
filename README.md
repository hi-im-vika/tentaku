# tentaku-mod

TENTAKU is a calculator with a numpad layout that uses mechanical keyswitches.

## Info
- [ATMEGA328P](https://www.ti.com/lit/ug/spmu298e/spmu298e.pdf) used as main controller
- [TM1638](https://futuranet.it/futurashop/image/catalog/data/Download/TM1638_V1.3_EN.pdf) used to control LEDs and 7-segment displays

## Building and Flashing
### DISCLAIMER
Please flash at your own risk. I'm working on this as a side project and I can't guarantee that this won't mess up your board. I'm not responsible for anything (good or bad) that happens as a result of this code. If you don't feel 100% comfortable, **DO NOT FLASH THIS FIRMWARE**.


Make sure you make a backup first by writing out the flash memory to a file format of your choose with a command like:

`avrdude -p m328p -P usb -c usbtiny -U flash:r:flash.hex:i` (for Intel HEX)

`avrdude -p m328p -P usb -c usbtiny -U flash:r:flash.bin:r` (for raw binary)

To build your own file, just run `make` in the project root. The output is in `./build/main.hex`.
You should know how to flash your file if you've perfomed a backup, so I'm leaving this as an exercise for the reader.

Please feel free to open an issue if you have problems.

## Registers
- Three registers: DDRD, PORTD, PIND, X is bit
- DDRDX controls data direction (sets input or output)
- PORTDX is written to when data needs to be sent
- PINDX is read from when data is received

### Pins (Bits)
- 5 (PORTD5) - DIO (data input/out)
- 6 (PORTD6) - CLK (clock)
- 7 (PORTD7) - STB (strobe/chip select)

### ICSP Headers
Listed from left to right with the board upright:
- ~CS (PORTB2)
- SCK (PORTB5)
- MISO (PORTB4)
- MOSI (PORTB3)
- GND
- VCC
