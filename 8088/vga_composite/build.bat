@echo off
yasm vga_composite.asm -o v.bin -l v.lst
rem copy /b v.bin+smpte_904_262.raw vga_composite.bin
rem copy /b v.bin vga_composite.bin
copy /b v.bin+frequencies.raw vga_composite.bin
copy /b v.bin+\smpte_900_262_22.raw vga_composite_22.bin
copy /b v.bin+\smpte_900_262_10.raw vga_composite_10.bin
copy /b v.bin+\smpte_900_262_true.raw vga_composite_true.bin

