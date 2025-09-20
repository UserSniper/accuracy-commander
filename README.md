# Accuracy Commander
A program for Commander X16 that tests the accuracy of your emulation/clone! \
Inspired by 100thCoin's Accuracy Coin.

## Fail Codes:
### SysROM is not writeable
0: System ROM was written to.

### The Decimal Flag
0: D Flag + ADC is not emulated correctly. \
1: D Flag + SBC is not emulated correctly.

### Indexed Address Timing
#### Pass:
1: 65c02 behavior.
2: 65c816 behavior.
#### Fail:
0: The timing is incorrect. \
1: Bit Shift Indexed on 65c02 is running as if it were on 65c816. \
2: Bit Shift Indexed on 65c816 is running as if it were on 65c02. 

### Addr Wrap: abs,x
1: LDA abs,x should cross page boundaries, but doesn't. \
2: LDA abs,x should cross from page $ff to $00, but doesn't. \
3: LDA abs,y should cross from page $ff to $00, but doesn't.

### Addr Wrap: zp,x
1: LDA zp,x does not function correctly. \
2: LDA zp,x does not wrap around the zeropage. \
3: LDX zp,y does not wrap around the zeropage.

### Addr Wrap: (indirect)
1: JMP (indirect) does not function correctly. \
2: JMP (indirect) should read the high byte from the same page of memory, but doesn't. 

### Addr Wrap: (indirect, x)
1: LDA (indirect, x) does not function correctly. \
2: LDA (indirect, x) should be confined to the zeropage, but isn't. \
3: LDA (indirect, x) should wrap around the page when reading low and high bytes, but doesn't.

### Addr Wrap: (indirect), y
1: LDA (indirect), y does not function correctly. \
2: The Y indexing should be allowed to cross page boundaries, but isn't. \
3: The address bus should wrap on a page boundary when reading low and high bytes, but doesn't.

### (65c02): Open Bus
0: Open Bus does not return the correct value. \
1: Indexed addressing crossing a page boundary updates the data bus, where it shouldn't.

### (65c02): All NOPs
1: One or more of the 1 byte, 1 cycle NOPs takes an incorrect amount of cycles. \
2. One or more of the 2 byte, 2 cycle NOPs takes an incorrect amount of cycles. \
3. One or more of the 2 byte, 2 cycle NOPs has an incorrect number of operands. \
4. The 2 byte, 3 cycle NOP ($44) has an incorrect amount of cycles. \
5. The 2 byte, 3 cycle NOP ($44) has an incorrect number of operands. \
6. One or more of the 2 byte, 4 cycle NOPs takes an incorrect amount of cycles. \
7. One or more of the 2 byte, 4 cycle NOPs has an incorrect number of operands. \
8. One or more of the 3 byte, 2 cycle NOPs takes an incorrect amount of cycles. \
9. One or more of the 3 byte, 2 cycle NOPs has an incorrect number of operands. \
a. The 3 byte, 8 cycle NOP ($5c) takes an incorrect amount of cycles. \
b. The 3 byte, 8 cycle NOP ($5c) has an incorrect number of operands.

### IO5-IO7 Operate at 2mhz
0: IO5-7 take the same amount of time (or longer) to read than IO3-4.



### VERA <$1:f9c0 reads VRAM
1: Reading the PSG does not read VRAM. \
2: Reading Color RAM does not read VRAM. \
3: Reading Sprite RAM does not read VRAM.

### VERA DCSEL behavior
0: Unused slots in VERA DCSEL should return the version number, but don't.
