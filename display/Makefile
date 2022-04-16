display.bin: display.elf
	arm-none-eabi-objcopy -Obinary display.elf display.bin
display.elf: display.o vectors.o startup.o ../common/trinket.ld
	arm-none-eabi-ld -Map display.map -T../common/trinket.ld -o display.elf startup.o display.o vectors.o -print-memory-usage -nostdlib
	arm-none-eabi-objdump -t display.elf
startup.o: ../common/startup.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb ../common/startup.s -o startup.o
vectors.o: ../common/vectors.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb  ../common/vectors.s -o vectors.o
display.o: display.c
	arm-none-eabi-gcc  -Wall  -g -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c display.c -o display.o
clean:
	rm startup.o display.elf display.bin display.o display.map vectors.o
debug:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg -c "program display.bin exit 0x00000000 verify reset exit"
gdb:
	arm-none-eabi-gdb -ex "target remote localhost:3333" display.elf
dump:
	hexdump -C display.bin
