spirainbow.bin: spirainbow.elf
	arm-none-eabi-objcopy -Obinary spirainbow.elf spirainbow.bin
spirainbow.elf: spi.o spirainbow.o vectors.o startup.o ../common/trinket.ld
	arm-none-eabi-ld -Map spirainbow.map -T../common/trinket.ld -o spirainbow.elf startup.o spi.o spirainbow.o vectors.o -print-memory-usage -nostdlib
	arm-none-eabi-objdump -t spirainbow.elf
startup.o: ../common/startup.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb ../common/startup.s -o startup.o
vectors.o: ../common/vectors.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb  ../common/vectors.s -o vectors.o
spirainbow.o: spirainbow.c
	arm-none-eabi-gcc -std=gnu99 -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c spirainbow.c -o spirainbow.o
spi.o: ../common/spi.c
	arm-none-eabi-gcc -std=gnu99 -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../common/spi.c -o spi.o
clean:
	rm startup.o spirainbow.elf spirainbow.bin spirainbow.o spirainbow.map vectors.o spi.o
debug:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg -c "program spirainbow.bin exit 0x00000000 verify reset exit"
gdb:
	arm-none-eabi-gdb -ex "target remote localhost:3333" spirainbow.elf
dump:
	hexdump -C spirainbow.bin
