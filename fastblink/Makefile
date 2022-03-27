fastblink.bin: fastblink.elf
	arm-none-eabi-objcopy -Obinary fastblink.elf fastblink.bin
fastblink.elf: fastblink.o vectors.o startup.o ../common/trinket.ld
	arm-none-eabi-ld -Map fastblink.map -T../common/trinket.ld -o fastblink.elf startup.o fastblink.o vectors.o -print-memory-usage -nostdlib
	arm-none-eabi-objdump -t fastblink.elf
startup.o: ../common/startup.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb ../common/startup.s -o startup.o
vectors.o: ../common/vectors.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb  ../common/vectors.s -o vectors.o
fastblink.o: fastblink.c
	arm-none-eabi-gcc  -Wall  -g -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c fastblink.c -o fastblink.o
clean:
	rm startup.o fastblink.elf fastblink.bin fastblink.o fastblink.map vectors.o
debug:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg -c "program fastblink.bin exit 0x00000000 verify reset exit"
gdb:
	arm-none-eabi-gdb -ex "target remote localhost:3333" fastblink.elf
dump:
	hexdump -C fastblink.bin
