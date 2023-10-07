WARN=-Wall
OPT=-Os

MCU=atmega328p
CPUFREQ=8000000
DEVINFO=-DF_CPU=$(CPUFREQ) -mmcu=$(MCU)

IDIR = ./include
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = $(DEVINFO) -I$(IDIR) $(WARN) $(OPT)

LIBS =

ODIR = src
BDIR = build

_DEPS = main.h buffer.h util.h stack.h calcfunc.h spi.h consts.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o buffer.o util.o stack.o calcfunc.o spi.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(BDIR)/flash.hex: $(BDIR)/main.elf
	$(OBJCOPY) -O ihex $(BDIR)/main.elf $(BDIR)/main.hex

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BDIR)/main.elf: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
