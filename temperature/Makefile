temperature.bin: temperature.elf
	arm-none-eabi-objcopy -Obinary temperature.elf temperature.bin
temperature.elf: i2c.o temperature.o vectors.o startup.o ../common/trinket.ld
	arm-none-eabi-ld -Map temperature.map -T../common/trinket.ld -o temperature.elf startup.o i2c.o temperature.o vectors.o -print-memory-usage -nostdlib
	arm-none-eabi-objdump -t temperature.elf
startup.o: ../common/startup.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb ../common/startup.s -o startup.o
vectors.o: ../common/vectors.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb  ../common/vectors.s -o vectors.o
temperature.o: temperature.c
	arm-none-eabi-gcc  -Wall  -g -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c temperature.c -o temperature.o
i2c.o: ../common/i2c.c
	arm-none-eabi-gcc  -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../common/i2c.c -o i2c.o
clean:
	rm startup.o temperature.elf temperature.bin temperature.o temperature.map vectors.o i2c.o
debug:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg -c "program temperature.bin exit 0x00000000 verify reset exit"
gdb:
	arm-none-eabi-gdb -ex "target remote localhost:3333" temperature.elf
dump:
	hexdump -C temperature.bin
