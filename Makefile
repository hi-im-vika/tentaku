BANNER = \n [0;38:5:93m██╗   ██[0;38:5:99m╗[0;38:5:63m██╗██╗  ██╗[0;38:5:69m [0;38:5:33m█████╗ ██[0;38:5:39m█╗   ███╗[0;38:5:38m █[0;38:5:44m████╗ ██╗ [0;38:5:43m [0;38:5:49m██╗███████[0;38:5:48m╗[0m\n [0;38:5:93m██║  [0;38:5:99m [0;38:5:63m██║██║██║ █[0;38:5:69m█[0;38:5:33m╔╝██╔══██[0;38:5:39m╗████╗ ██[0;38:5:38m██[0;38:5:44m║██╔══██╗█[0;38:5:43m█[0;38:5:49m║ ██╔╝██╔═[0;38:5:48m═══╝[0m\n [0;38:5:93m██[0;38:5:99m║[0;38:5:63m   ██║██║██[0;38:5:69m█[0;38:5:33m██╔╝ ████[0;38:5:39m███║██╔██[0;38:5:38m██[0;38:5:44m╔██║██████[0;38:5:43m█[0;38:5:49m║█████╔╝ █[0;38:5:48m████╗[0m\n [0;38:5:63m╚██╗ ██╔╝██[0;38:5:69m║[0;38:5:33m██╔═██╗ █[0;38:5:39m█╔══██║██[0;38:5:38m║╚[0;38:5:44m██╔╝██║██╔[0;38:5:43m═[0;38:5:49m═██║██╔═██[0;38:5:48m╗ ██╔══╝[0m\n [0;38:5:63m ╚████╔╝[0;38:5:69m [0;38:5:33m██║██║  █[0;38:5:39m█╗██║  ██[0;38:5:38m║█[0;38:5:44m█║ ╚═╝ ██║[0;38:5:43m█[0;38:5:49m█║  ██║██║[0;38:5:48m  ██╗████[0;38:5:84m█[0;38:5:83m██╗[0m\n [0;38:5:63m  ╚══[0;38:5:69m═[0;38:5:33m╝  ╚═╝╚═╝[0;38:5:39m  ╚═╝╚═╝ [0;38:5:38m ╚[0;38:5:44m═╝╚═╝     [0;38:5:43m╚[0;38:5:49m═╝╚═╝  ╚═╝[0;38:5:48m╚═╝  ╚═╝╚[0;38:5:84m═[0;38:5:83m═════╝\033[0m\n

WARN=-Wall
OPT=-Os

MCU=atmega328p
CPUFREQ=8000000
DEVINFO=-DF_CPU=$(CPUFREQ) -mmcu=$(MCU)

BUILDDIR=./build
SRCDIR=./src
LIBDIR=./lib

LIBSRC=$(wildcard $(LIBDIR)/*.c)
LIBOBJ=$(LIBSRC:.c=.o)

all:
	@# make build folder if it doesn't exist already
	@mkdir -p ./build
	@printf " $(BANNER)\n"
	avr-gcc $(WARN) $(OPT) $(DEVINFO) -c $(SRCDIR)/main.c -o $(BUILDDIR)/main.o
	avr-gcc $(WARN) $(DEVINFO) -o $(BUILDDIR)/main.elf $(BUILDDIR)/main.o
	avr-objcopy -O ihex $(BUILDDIR)/main.elf $(BUILDDIR)/main.hex

lib: $(LIBOBJ)
	@mkdir -p ./build
	avr-gcc $(WARN) $(OPT) $(DEVINFO) -c $^ -o $@

# makefile not yet updated to compile all files
# avr-gcc -Wall -Os -DF_CPU=8000000 -mmcu=atmega328p -c ./src/main.c -o ./build/main.o
# avr-gcc -Wall -Os -DF_CPU=8000000 -mmcu=atmega328p -c ./lib/stack.c -o ./build/stack.o
# avr-gcc -Wall -Os -DF_CPU=8000000 -mmcu=atmega328p -c ./lib/util.c -o ./build/util.o
# avr-gcc -Wall -Os -DF_CPU=8000000 -mmcu=atmega328p -c ./lib/spi.c -o ./build/spi.o
# avr-gcc -Wall -Os -DF_CPU=8000000 -mmcu=atmega328p -c ./lib/buffer.c -o ./build/buffer.o
# avr-gcc -Wall -DF_CPU=8000000 -mmcu=atmega328p -o ./build/main.elf ./build/*.o
# avr-objcopy -O ihex ./build/main.elf ./build/main.hex
