MCU := atmega328p
F_CPU := 16000000UL
PORT := /dev/cu.usbserial-2110
BAUD := 115200
TARGET := main
BUILD := .build
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c,$(BUILD)/%.o,$(SRC))

# Tools
CC := avr-gcc
OBJCOPY := avr-objcopy
AVRDUDE := avrdude

# Flags
CFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall
CFLAGS += -DAPP_VERSION_STRING=\"$(shell git describe --no-abbrev)\"

LDFLAGS := -mmcu=$(MCU)

# Outputs
ELF := $(BUILD)/$(TARGET).elf
HEX := $(BUILD)/$(TARGET).hex

.PHONY: all build flash clean size help

# Default target
all: build

build: | $(BUILD) $(HEX)

$(BUILD):
	@mkdir -p $(BUILD)

$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

# Flash to device
flash: $(HEX)
	$(AVRDUDE) -c arduino -p m328p -P $(PORT) -b $(BAUD) -U flash:w:$(HEX)

# Show size (optional if avr-size is available)
size: $(ELF)
	-@command -v avr-size >/dev/null 2>&1 && avr-size -C --mcu=$(MCU) $(ELF) || echo "avr-size not found"

# Clean artifacts
clean:
	rm -rf $(BUILD)

# Quick help
help:
	@echo "Makefile targets:"
	@echo "  all/build  - Build $(HEX) for $(MCU) @ $(F_CPU)"
	@echo "  flash      - Flash $(HEX) to device on $(PORT) @ $(BAUD)"
	@echo "  size       - Print binary size (if avr-size available)"
	@echo "  clean      - Remove build artifacts"
	@echo "Variables you can override, e.g.: make PORT=/dev/ttyACM0 BAUD=57600"
