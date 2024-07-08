NAME = shiftreg
TARGET = TARGET_STM32F4

SRC =  shiftreg.c systick.c gpio.c
OBJ = $(SRC:.c=.o)

CC = arm-none-eabi-gcc
ASM = arm-none-eabi-as

STATE_ENGINE_DIR = ../stateengine/src
COMMON_DIR = ../common

ASMFLAGS = -g -mcpu=cortex-m0plus -mthumb 

ifeq ($(TARGET), TARGET_STM32F4)
HAL_DIR = ../hal/stmf4
ASMFLAGS = -g -mcpu=cortex-m4 -mthumb 
TARGET_CPU = cortex-m4
else
HAL_DIR = ../common
TARGET_CPU = cortex-m0plus
endif

CFLAGS = -std=gnu11 -Wall  -g -I$(COMMON_DIR) -I$(HAL_DIR) -I../stateengine/src -nostdlib -nostartfiles -mcpu=$(TARGET_CPU) -mthumb -fdata-sections -ffunction-sections -D $(TARGET) -D TARGET_ARM
LDFLAGS = -print-memory-usage -nostdlib --gc-sections



STARTUP_STM32 = m0plus_vectors.o stm32_vectors.o startup.o
STARTUP_STM32F4 = m4_vectors.o target_vectors.o m4startup.o
STARTUP_SAMD21 =  ../common/m0plus_vectors.o ../common/samd21_vectors.o ../common/startup.o

$(NAME).bin: $(NAME).elf
	arm-none-eabi-objcopy -Obinary shiftreg.elf shiftreg.bin

ifeq ($(TARGET), TARGET_STM32)
$(NAME).elf: $(OBJ) $(STARTUP_STM32) $(HAL_DIR)/stm32.ld
	echo "STM32 Target Selected"
	arm-none-eabi-ld -Map $(NAME).map -T$(HAL_DIR)/stm32.ld -o $(NAME).elf $(OBJ) $(STARTUP_STM32) $(LDFLAGS)

else ifeq ($(TARGET), TARGET_STM32F4)
$(NAME).elf: $(OBJ) $(STARTUP_STM32F4) $(HAL_DIR)/memory.ld
	echo "STM32F4 Target Selected"
	arm-none-eabi-ld -Map $(NAME).map -T$(HAL_DIR)/memory.ld -o $(NAME).elf $(OBJ) $(STARTUP_STM32F4) $(LDFLAGS)

else
$(NAME).elf: $(OBJ) $(STARTUP_SAMD21) m0.ld
	echo "Defaulting to SAMD21"
	arm-none-eabi-ld -Map $(NAME).map -Tm0.ld -o $(NAME).elf $(OBJ) $(STARTUP_SAMD21) $(LDFLAGS)
endif
%.o : $(COMMON_DIR)/%.s
	$(ASM) -c $(ASMFLAGS) -o $@ $<
%.o : $(HAL_DIR)/%.s
	$(ASM) -c $(ASMFLAGS) -o $@ $<
%.o : $(HAL_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.o : $(STATE_ENGINE_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.o : $(COMMON_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<
clean:
	rm *.o *.elf *.map *.bin
debug:
ifeq ($(TARGET), TARGET_STM32)
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32g0x.cfg
else
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32f4x.cfg
endif
flash:
ifeq ($(TARGET), TARGET_STM32)
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32g0x.cfg -c "program shiftreg.bin exit 0x08000000 verify reset"
else
	openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f /usr/share/openocd/scripts/target/stm32f4x.cfg -c "program shiftreg.bin exit 0x08000000 verify reset"
endif
gdb:
	arm-none-eabi-gdb -ex "target extended-remote localhost:3333" shiftreg.elf
dump:
	hexdump -C shiftreg.bin
objdump:
	arm-none-eabi-objdump -t shiftreg.elf
