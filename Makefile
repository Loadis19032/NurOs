gcc = i686-elf-gcc
ld = i686-elf-ld
flags = -c -ffreestanding

all: kernel boot assemble

kernel:
	### Kernel #####
	$(gcc) $(flags) src/kernel//kernel.c -o build/kernel.o
	$(gcc) $(flags) src/lib/stdlib/stdio.c -o build/stdio.o
	$(gcc) $(flags) src/drivers/vga/vga.c -o build/vga.o
	$(gcc) $(flags) src/kernel/gdt/gdt.c -o build/gdt.o
	$(gcc) $(flags) src/kernel/timer/timer.c -o build/timer.o
	$(gcc) $(flags) src/lib/util.c -o build/util.o
	$(gcc) $(flags) src/kernel/idt/idt.c -o build/idts.o
	$(gcc) $(flags) src/drivers/keyboard/keyboard.c -o build/keyboard.o
	$(gcc) $(flags) src/kernel/mem/malloc.c -o build/malloc.o
	$(gcc) $(flags) src/lib/mem/mem.c -o build/mem.o	
	$(gcc) $(flags) src/kernel/fs/vfs/vfs.c -o build/vfs.o
	$(gcc) $(flags) src/lib/stdlib/string.c -o build/string.o
	$(gcc) $(flags) src/kernel/fs/fat32/fat.c -o build/fat.o
	$(gcc) $(flags) src/drivers/disk/ata.c -o build/ata.o

boot:
	## bootloader ###
	nasm -f elf32 src/kernel/boot/boot.s -o build/boot.o
	nasm -f elf32 src/kernel/gdt/gdt.s -o build/gdts.o
	nasm -f elf32 src/kernel/idt/idt.s -o build/idt.o

	### else #####

assemble:
	$(ld) -T linker.ld -o kernel build/boot.o build/kernel.o build/vga.o build/gdts.o build/gdt.o build/idts.o build/idt.o  build/util.o build/timer.o build/stdio.o build/keyboard.o build/malloc.o build/mem.o build/string.o build/vfs.o build/fat.o build/ata.o
	mv kernel src/nuros/boot/kernel
	dd if=/dev/zero of=nuros.iso bs=1M count=100
	mkfs.fat -F32 nuros.iso
	#sudo losetup /dev/loop0 nuros.iso
	#mkdir /mnt
	#mkdir /mnt/disk
	#mkdir /mnt/disk/boot
	#sudo mount /dev/loop0 /mnt/disk
	#sudo grub-install --force --target=i386-pc --boot-directory=/mnt/disk/boot/ /dev/loop0
	#cp -r src/nuros/boot/kernel /mnt/disk/boot
	#sudo cp src/nuros/boot/grub/grub.cfg /mnt/disk/boot
	#mkdir /mnt/disk/bin
	#mkdir /mnt/disk/usr
	#umount /mnt/disk
	#losetup -d /dev/loop0
	#rm -rf /mnt/disk
	##qemu-system-x86_64 -drive format=raw,file=nuros.iso  
	grub-mkrescue -o build/nuros.iso src/nuros/
	qemu-system-i386 build/nuros.iso
