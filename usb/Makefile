usb.bin: usb.elf
	arm-none-eabi-objcopy -Obinary usb.elf usb.bin
usb.elf: clock.o fsm.o usb.o vectors.o startup.o ../common/trinket.ld
	arm-none-eabi-ld -Map usb.map -T../common/trinket.ld -o usb.elf startup.o clock.o fsm.o usb.o vectors.o -print-memory-usage -nostdlib
	arm-none-eabi-objdump -t usb.elf
startup.o: ../common/startup.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb ../common/startup.s -o startup.o
vectors.o: ../common/vectors.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb  ../common/vectors.s -o vectors.o
usb.o: usb.c
	arm-none-eabi-gcc -std=gnu99 -Wall  -g -nostdlib -nostartfiles -I../common -mcpu=cortex-m0plus -mthumb -c usb.c -o usb.o
fsm.o: ../stateengine/src/fsm.c
	arm-none-eabi-gcc -std=gnu99 -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../stateengine/src/fsm.c -o fsm.o
clock.o: ../common/clock.c
	arm-none-eabi-gcc -std=gnu99 -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../common/clock.c -o clock.o
clean:
	rm startup.o usb.elf usb.bin usb.o usb.map vectors.o fsm.o clock.o
debug:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg -c "program usb.bin exit 0x00000000 verify reset exit"
gdb:
	arm-none-eabi-gdb -ex "target remote localhost:3333" usb.elf
dump:
	hexdump -C usb.bin
