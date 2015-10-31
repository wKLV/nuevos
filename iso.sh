#!/bin/sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp build-tmp/nuevos.bin isodir/boot/nuevos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "nuevos" {
	multiboot /boot/nuevos.kernel
}
EOF
grub-mkrescue -o nuevos.iso isodir
