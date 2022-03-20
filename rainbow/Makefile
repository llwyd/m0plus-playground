rainbow.bin: rainbow.elf
	arm-none-eabi-objcopy -Obinary rainbow.elf rainbow.bin
rainbow.elf: rainbow.o timer.o buffer.o vectors.o startup.o ../common/trinket.ld
	arm-none-eabi-ld -Map rainbow.map -T../common/trinket.ld -o rainbow.elf startup.o buffer.o timer.o rainbow.o vectors.o -print-memory-usage -nostdlib
	arm-none-eabi-objdump -t rainbow.elf
startup.o: ../common/startup.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb ../common/startup.s -o startup.o
vectors.o: ../common/vectors.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb  ../common/vectors.s -o vectors.o
rainbow.o: rainbow.c
	arm-none-eabi-gcc  -Wall  -g -nostdlib -nostartfiles -mcpu=cortex-m0plus -I../common -mthumb -c rainbow.c -o rainbow.o
buffer.o: ../common/buffer.c
	arm-none-eabi-gcc  -Wall  -g -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../common/buffer.c -o buffer.o
timer.o: timer.c
	arm-none-eabi-gcc  -Wall  -g -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c timer.c -o timer.o
clean:
	rm startup.o rainbow.elf rainbow.bin rainbow.o rainbow.map vectors.o  timer.o buffer.o
debug:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg -c "program rainbow.bin exit 0x00000000 verify reset exit"
gdb:
	arm-none-eabi-gdb -ex "target remote localhost:3333" rainbow.elf
dump:
	hexdump -C rainbow.bin
