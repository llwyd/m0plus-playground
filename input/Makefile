input.bin: input.elf
	arm-none-eabi-objcopy -Obinary input.elf input.bin
input.elf: input.o vectors.o startup.o ../common/trinket.ld
	arm-none-eabi-ld -Map input.map -T../common/trinket.ld -o input.elf startup.o input.o vectors.o -print-memory-usage -nostdlib
	arm-none-eabi-objdump -t input.elf
startup.o: ../common/startup.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb ../common/startup.s -o startup.o
vectors.o: ../common/vectors.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb  ../common/vectors.s -o vectors.o
input.o: input.c
	arm-none-eabi-gcc  -Wall  -g -nostdlib -nostartfiles -I../common -mcpu=cortex-m0plus -mthumb -c input.c -o input.o
clean:
	rm startup.o input.elf input.bin input.o input.map vectors.o
debug:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg -c "program input.bin exit 0x00000000 verify reset exit"
gdb:
	arm-none-eabi-gdb -ex "target remote localhost:3333" input.elf
dump:
	hexdump -C input.bin
