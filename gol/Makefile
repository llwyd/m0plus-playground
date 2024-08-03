NAME = conway
TARGET = TARGET_STM32

SRC =  conway.c events.c state.c clock.c life.c timer.c i2c.c display.c systick.c
OBJ = $(SRC:.c=.o)

CC = arm-none-eabi-gcc
ASM = arm-none-eabi-as

HAL_DIR = ../hal/stm32g0
SRCDIR = ../common
STATE_ENGINE_DIR = ../boilerplate/src
LIFE_DIR = ../conway/life

CFLAGS = -std=gnu11 -Wall -Wfatal-errors -c -O3 -g -I../common -I../boilerplate/src -ffreestanding -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -fdata-sections -ffunction-sections -D $(TARGET) -D TARGET_ARM
ASMFLAGS = -g -mcpu=cortex-m0plus -mthumb 
LDFLAGS = -print-memory-usage -nostdlib --gc-sections

COMMON_DIR = ../common

STARTUP_STM32 = m0plus_vectors.o stm32_vectors.o startup.o

$(NAME).bin: $(NAME).elf
	arm-none-eabi-objcopy -Obinary conway.elf conway.bin
$(NAME).elf: $(OBJ) $(STARTUP_STM32) ../common/stm32.ld
	arm-none-eabi-ld -Map $(NAME).map -T../common/stm32.ld -o $(NAME).elf $(OBJ) $(STARTUP_STM32) $(LDFLAGS)

%.o : $(COMMON_DIR)/%.s
	$(ASM) -c $(ASMFLAGS) -o $@ $<
%.o : $(HAL_DIR)/%.s
	$(ASM) -c $(ASMFLAGS) -o $@ $<
%.o : $(HAL_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.o : $(STATE_ENGINE_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<
%.o : $(LIFE_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<
%.o : $(COMMON_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<
clean:
	rm *.o *.elf *.map *.bin
debug:
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32g0x.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32g0x.cfg -c "program conway.bin exit 0x08000000 verify reset"
gdb:
	arm-none-eabi-gdb -ex "target extended-remote localhost:3333" conway.elf
dump:
	hexdump -C conway.bin
objdump:
	arm-none-eabi-objdump -t conway.elf
