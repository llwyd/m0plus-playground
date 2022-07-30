conway.bin: conway.elf
	arm-none-eabi-objcopy -Obinary conway.elf conway.bin
conway.elf: fsm.o clock.o i2c.o display.o adc.o life.o conway.o vectors.o startup.o ../common/trinket.ld
	arm-none-eabi-ld -Map conway.map -T../common/trinket.ld -L /usr/lib/gcc/arm-none-eabi/12.1.0/ -o conway.elf startup.o fsm.o clock.o i2c.o display.o adc.o life.o conway.o vectors.o -print-memory-usage -nostdlib -lgcc 
	arm-none-eabi-objdump -t conway.elf
startup.o: ../common/startup.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb ../common/startup.s -o startup.o
vectors.o: ../common/vectors.s
	arm-none-eabi-as -g -mcpu=cortex-m0plus -mthumb  ../common/vectors.s -o vectors.o
conway.o: conway.c
	arm-none-eabi-gcc -std=gnu11 -Ofast -Wall  -g -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c conway.c -o conway.o
life.o: ../../../conway/life/life.c
	arm-none-eabi-gcc -std=gnu11 -Ofast -Wall  -g -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../../../conway/life/life.c -o life.o -ffreestanding
fsm.o: ../stateengine/src/fsm.c
	arm-none-eabi-gcc -std=gnu11 -Ofast -Wall  -g -D TARGET_ARM -I../stateengine/src -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../stateengine/src/fsm.c -o fsm.o
adc.o: ../common/adc.c
	arm-none-eabi-gcc -std=gnu11 -Ofast -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../common/adc.c -o adc.o
display.o: ../common/display.c
	arm-none-eabi-gcc -std=gnu11 -Ofast -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../common/display.c -o display.o
i2c.o: ../common/i2c.c
	arm-none-eabi-gcc -std=gnu11 -Ofast -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../common/i2c.c -o i2c.o
clock.o: ../common/clock.c
	arm-none-eabi-gcc -std=gnu11 -Ofast -Wall  -g -I../common -nostdlib -nostartfiles -mcpu=cortex-m0plus -mthumb -c ../common/clock.c -o clock.o
clean:
	rm startup.o conway.elf conway.bin conway.o conway.map vectors.o i2c.o life.o clock.o fsm.o display.o adc.o
debug:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg
flash:
	openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -c "transport select swd" -f /usr/share/openocd/scripts/target/at91samdXX.cfg -c "program conway.bin exit 0x00000000 verify reset exit"
gdb:
	arm-none-eabi-gdb -ex "target remote localhost:3333" conway.elf
dump:
	hexdump -C conway.bin
