NAME = blink
TARGET = TARGET_STM32

SRC =  blink.c fsm.c clock.c
OBJ = $(SRC:.c=.o)

CC = arm-none-eabi-gcc
ASM = arm-none-eabi-as

SRCDIR = ../common

CFLAGS = -std=gnu11 -Wall  -g -I../common -I../stateengine/src -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -fdata-sections -ffunction-sections -D $(TARGET) -D TARGET_ARM
ASMFLAGS = -g -mcpu=cortex-m0plus -mthumb 
LDFLAGS = -print-memory-usage -nostdlib --gc-sections

STATE_ENGINE_DIR = ../stateengine/src
COMMON_DIR = ../common

STARTUP_STM32 = m0plus_vectors.o stm32_vectors.o startup.o
STARTUP_SAMD21 =  ../common/m0plus_vectors.o ../common/samd21_vectors.o ../common/startup.o

$(NAME).bin: $(NAME).elf
	arm-none-eabi-objcopy -Obinary blink.elf blink.bin
ifeq ($(TARGET), TARGET_STM32)
$(NAME).elf: $(OBJ) $(STARTUP_STM32) ../common/stm32.ld
	echo "STM32 Target Selected"
	arm-none-eabi-ld -Map $(NAME).map -T../common/stm32.ld -o $(NAME).elf $(OBJ) $(STARTUP_STM32) $(LDFLAGS)
else
$(NAME).elf: $(OBJ) $(STARTUP_SAMD21) m0.ld
	echo "Defaulting to SAMD21"
	arm-none-eabi-ld -Map $(NAME).map -Tm0.ld -o $(NAME).elf $(OBJ) $(STARTUP_SAMD21) $(LDFLAGS)
endif
%.o : $(COMMON_DIR)/%.s
	$(ASM) -c $(ASMFLAGS) -o $@ $<
%.o : $(STATE_ENGINE_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.o : $(COMMON_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<
clean:
	rm *.o *.elf *.map *.bin
debug:
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32g0x.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32g0x.cfg -c "program blink.bin exit 0x08000000 verify reset"
gdb:
	arm-none-eabi-gdb -ex "target extended-remote localhost:3333" blink.elf
dump:
	hexdump -C blink.bin
objdump:
	arm-none-eabi-objdump -t blink.elf
