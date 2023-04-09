# tentaku-mod

TENTAKU is a calculator with a numpad layout that uses mechanical keyswitches.

## Info
- ATMEGA328P used as main controller
- TM1638 used to control LEDs and 7-segment displays

## Registers
- Three registers: DDRD, PORTD, PIND, X is bit
- DDRDX controls data direction (sets input or output)
- PORTDX is written to when data needs to be sent
- PINDX is read from when data is received

### Pins (Bits)
- 5 - DIO (data input/out)
- 6 - CLK (clock)
- 7 - STB (strobe/chip select)
