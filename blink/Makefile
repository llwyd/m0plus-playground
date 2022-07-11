blink.bin: blink.elf
	arm-none-eabi-objcopy -Obinary blink.elf blink.bin
blink.elf: fsm.o blink.o vectors.o startup.o ../common/trinket.ld
	arm-none-eabi-ld -Map blink.map -T../common/trinket.ld -o blink.elf startup.o fsm.o blink.o vectors.o -print-memory-usage -nostdlib
	arm-none-eabi-objdump -t blink.elf
startup.o: ../common/startup.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb ../common/startup.s -o startup.o
vectors.o: ../common/vectors.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb  ../common/vectors.s -o vectors.o
blink.o: blink.c
	arm-none-eabi-gcc -std=gnu99 -Wall  -g -nostdlib -nostartfiles -I../common -mcpu=cortex-m0plus -mthumb -c blink.c -o blink.o
fsm.o: ../common/fsm.c
	arm-none-eabi-gcc -std=gnu99 -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../common/fsm.c -o fsm.o
clean:
	rm startup.o blink.elf blink.bin blink.o blink.map vectors.o fsm.o
debug:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg -c "program blink.bin exit 0x00000000 verify reset exit"
gdb:
	arm-none-eabi-gdb -ex "target remote localhost:3333" blink.elf
dump:
	hexdump -C blink.bin
