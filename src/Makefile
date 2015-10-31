all:
	cd .. && \
	mkdir -p build-tmp && \
	i686-elf-as src/boot.s -o build-tmp/boot.o && \
	i686-elf-gcc -c src/kernel.c -o build-tmp/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra && \
	i686-elf-gcc -T src/linker.ld -o build-tmp/nuevos.bin -ffreestanding -O2 -nostdlib build-tmp/boot.o build-tmp/kernel.o -lgcc && \
	./iso.sh && \
	rm -r ../build-tmp
