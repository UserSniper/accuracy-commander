CC = mos-cx16-clang
CARGS = -Oz -flto -fnonreentrant -Wno-int-conversion 



default: make






make:
	rm -rf ./OUT
	mkdir ./OUT
	$(CC) $(CARGS) main.c -o ./OUT/BRUH.PRG

	cd ./OUT
	x16emu -debug -zeroram -midline-effects -c02 -prg ./OUT/BRUH.PRG -run