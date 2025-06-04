NAME = pq
TARGET = TARGET_STM32

SRC =  pq.c state.c fifo_base.c events.c event_heap.c systick.c
OBJ = $(SRC:.c=.o)

CC = arm-none-eabi-gcc
ASM = arm-none-eabi-as

HAL_DIR = ../hal/stm32l432
SRCDIR = ../common
BOILERPLATE_DIR = ../boilerplate/src
OPT = -O0

CFLAGS = -std=gnu11 -Wall -Wfatal-errors -c $(OPT) -g -I$(SRCDIR) -I$(HAL_DIR) -I$(BOILERPLATE_DIR) -ffreestanding -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -fdata-sections -ffunction-sections -D $(TARGET) -D TARGET_ARM -D NDEBUG
ASMFLAGS = -g -mcpu=cortex-m4 -mthumb 
LDFLAGS = -print-memory-usage -nostdlib --gc-sections

COMMON_DIR = ../common

STARTUP_STM32 = m4_vectors.o target_vectors.o m4startup.o

$(NAME).bin: $(NAME).elf
	arm-none-eabi-objcopy -Obinary pq.elf pq.bin
$(NAME).elf: $(OBJ) $(STARTUP_STM32) $(HAL_DIR)/memory.ld
	arm-none-eabi-ld -Map $(NAME).map -T$(HAL_DIR)/memory.ld -o $(NAME).elf $(OBJ) $(STARTUP_STM32) $(LDFLAGS)

%.o : $(COMMON_DIR)/%.s
	$(ASM) -c $(ASMFLAGS) -o $@ $<
%.o : $(HAL_DIR)/%.s
	$(ASM) -c $(ASMFLAGS) -o $@ $<
%.o : $(HAL_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.o : $(BOILERPLATE_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<
%.o : $(COMMON_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<
clean:
	rm *.o *.elf *.map *.bin
debug:
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32l4x.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32l4x.cfg -c "program pq.bin exit 0x08000000 verify reset"
gdb:
	arm-none-eabi-gdb -ex "target extended-remote localhost:3333" pq.elf
dump:
	hexdump -C pq.bin
objdump:
	arm-none-eabi-objdump -t pq.elf
