const char * test_values[] = {
    "Test",
    "Pass",
    "Fail"
};

const char * test_names[] = {
    "SysROM is not writeable", // 00
    "The Decimal Flag",
    "Bit Shift abs,x timing",
    "Addr Wrap:   abs indexed",
    "Addr Wrap:    zp indexed",
    "Addr Wrap:    (indirect)",
    "Addr Wrap: (indirect, x)",
    "Addr Wrap: (indirect), y", 
    "65c02: Open Bus", // 08
    "65c02: All NOPs",
    "IO5-IO7 operate at 2mhz",
    "",
    "",
    "",
    "",
    "",

    "VERA >$1:f9c0 reads VRAM", // 10
    "VERA DCSEL behavior",
    "VERA FX",
    "",
    "",
    "",
    "",
    "",
    //" ", // 18
};

#define test_result ((unsigned char*)0x0400)

unsigned char selection;


void wait_for_line_256(){
    __attribute__((leaf)) __asm__ volatile (
        // wait for next frame. who knows where
        // the beam is right now
        "wai \n"

        // wait for line 256
        "lda $9f26 \n"
        "and #$40 \n"
        "bne $F9 \n" // -7 bytes

        :
        :
        :"a","x","y","p"
    );
}

void draw_test_names(unsigned char offset){

    for (char i=offset; i < (offset+16); i++){
        cbm_k_plot(7, (4+(i-offset)));
        printf("%24s",test_names[i]);
    }
}

void draw_test_select(unsigned char offset){
    unsigned char tmp;
    for (char i=offset; i < (offset+16); i++){

        cbm_k_plot(1, (4+(i-offset)));


        tmp = test_result[i];
        if(tmp < 0x80){
            switch(tmp){
                case 0: putchar(0x05); break;
                case 1: putchar(0x99); break;
                default: {
                    putchar(0x96); 
                    tmp = 2;
                    break;
                }
            }
        } else {
            putchar(0x99);
            tmp = 1;
        }

        

        if(selection != i) printf("%s",test_values[tmp]);
        else printf("\x01%s\x01",test_values[tmp]);

        if(test_result[i] >= 0x80) printf("%2x",(test_result[i]-0x80));
        else if(test_result[i] >= 2) printf("%2x",(test_result[i]-2));
        else printf("  ");

        putchar(0x05);
    }
}



#define POKE(addr,val)  ((*(unsigned char*)(addr)) = val)
#define PEEK(addr)      (*(unsigned char*)(addr))

#define TEST_PASS 1
#define TEST_FAIL 2
#define TEST_PASS_WC 0x80

unsigned char run_test(unsigned char test){
    unsigned char err_code = TEST_PASS;
    unsigned char tmp=0;
    unsigned char is_65816;
    // Pre-Test: what CPU are we running on?
    __attribute__((leaf)) __asm__ volatile(
        
        "ldx #$FF \n"
        "ldy #$00 \n"
        ".byte $9B ; TXY \n"
        // $9B does nothing on 65C02

        :"=y"(is_65816)
        :
        :"a","x","p"
    );

    switch(test){

        // Test: SysROM is not writeable 
        // Write to System ROM and see if it changed.
        case 0: {

            ROM_BANK = 0x1f;
            tmp = PEEK(0xffc0);

            POKE(0xffc0,(tmp+0x55));

            // if the value was written to ROM, this test fails.
            if (PEEK(0xffc0) == tmp) err_code = TEST_FAIL;
            
            ROM_BANK = 0x00;
            break;
        }

        // Test: The Decimal Flag
        // See if BCD works properly.
        case 1: {

            // Test 1: Figure out if ADC and SBC are
            // affected by the Decimal Flag.
            __attribute__((leaf)) __asm__ volatile (
                "sed \n"
                "lda #$09 \n"
                "clc \n"
                "adc #$01 \n"
                "cld"
                : "=a"(tmp)
                :
                : "x", "y", "p"
            );
            
            if(tmp != 0x10) { // D+ADC is not emulated correctly
                err_code = TEST_FAIL;
                //cbm_k_plot(1,22);
                //printf("adc result: %2x", tmp);
            } 
            

            __attribute__((leaf)) __asm__ volatile (
                "sed \n"
                "lda #$10 \n"
                "sec \n"
                "sbc #$01 \n"
                "cld"
                : "=a"(tmp)
                :
                : "x", "y", "p"
            );
            
            if(tmp != 0x09) { // D+SBC is not emulated correctly
                err_code = TEST_FAIL+1;
                //cbm_k_plot(1,23);
                //printf("sbc result: %2x", tmp); 
            }

            break;
        }

        // Test: Indexed Address timing
        // Bit Shift abs,x / abs,y takes 1 cycle longer on 
        // 65C816, so test for that here.
        case 2: {
            // Test 1: Check how many ASL,x instructions
            // can be executed in a specified amount of time.
            __attribute__((leaf)) __asm__ volatile(

                // setup
                "ldx #$26 \n"
                "ldy #$00 \n"
                "inc $00 \n"

                // wait for next frame. who knows where
                // the beam is right now
                "wai \n"

                // wait for line 256
                "indexed_address_wait_for_secondhalf: \n"
                "lda $9f26 \n"
                "and #$40 \n"
                "bne indexed_address_wait_for_secondhalf \n"

                // it's more reliable to wait for line 256
                // than to start from the end of vsync.

                "indexed_address_run_the_test: \n"
                "asl $a000,x \n" // 
                "lsr $a000,x \n" // these takes longer on 65816
                "rol $a000,x \n" //
                "ror $a000,x \n" //
                "iny \n"
                "lda $9f26 \n"
                "and #$40 \n"
                "beq indexed_address_run_the_test \n"
                
                "dec $00"
                :"=y"(tmp) // Y will return 8 on C02, and 7 on C816
                :
                :"a","x","p"
            );
            
            if(tmp == 0x8){ // 65C02 behavior
                if(is_65816) {err_code = TEST_FAIL+1;}
                else {err_code = TEST_PASS_WC+1;}
                break;
            }

            if(tmp == 0x7){ // 65C816 behavior
                if(is_65816) {err_code = TEST_PASS_WC+2;}
                else {err_code = TEST_FAIL+2;}
                break;
            }

            err_code = TEST_FAIL;
            break;
        }

        // Test: Addressing Mode Wraparound: Absolute Indexed
        case 3: {
            RAM_BANK = 0x01;

            // Test 1: Does LDA Absolute, X read from the expected address?
            // crossing a page boundary:
            __attribute__((leaf)) __asm__ volatile(
                // store 0x5A in $01:a180
                "lda #$5a \n"
                "sta $a180 \n"
                "ldx #$F0 \n"

                "lda $a090,x \n" // read from $01:a180

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if(tmp != 0x5a){
                err_code = TEST_FAIL+1;
                break;
            }


            // Test 2: Wrapping around from $ffff to $0000
            __attribute__((leaf)) __asm__ volatile(
                // store #$5a in $00fe
                "lda #$5a \n"
                "sta $fe \n"
                "ldx #$ff \n"

                "lda $ffff,x \n" // read from $00fe

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if(tmp != 0x5a){
                err_code = TEST_FAIL+2;
                break;
            }
            // Running Test 2 again, this time with a different value.
            __attribute__((leaf)) __asm__ volatile(
                // store #$5a in $00fe
                "lda #$a5 \n"
                "sta $fe \n"
                "ldx #$ff \n"

                "lda $ffff,x \n" // read from $00fe

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if(tmp != 0xa5){
                err_code = TEST_FAIL+2;
                break;
            }

            // Running Test 2 again, this time using Y.
            __attribute__((leaf)) __asm__ volatile(
                "ldy #$ff \n"

                "lda $ffff,y \n" // read from $00fe

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if(tmp != 0xa5){
                err_code = TEST_FAIL+3;
                break;
            }

            break;
        }

        // Test: Addressing Mode Wraparound: Zeropage Indexed
        case 4: {
            RAM_BANK = 0x01;

            // Test 1: Does LDA Zeropage, X read from the expected address?
            __attribute__((leaf)) __asm__ volatile(
                // store 0x5A in $fe
                "lda #$5a \n"
                "sta $fe \n"
                "ldx #$0e \n"

                "lda $f0,x \n" // read from $fe

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if(tmp != 0x5a){
                err_code = TEST_FAIL+1;
                break;
            }

            // Test 2: Zeropage Indexed should always remain in the zeropage.
            __attribute__((leaf)) __asm__ volatile(
                "ldx #$ff \n"

                "lda $ff,x \n" // read from $fe

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if(tmp != 0x5a){
                err_code = TEST_FAIL+2;
                break;
            }

            // Running Test 2 again, this time with Y.
            __attribute__((leaf)) __asm__ volatile(
                "ldy #$ff \n"

                // i almost missed this:
                // LDA does not have a zp,y variant.
                // use LDX zp,y instead.
                "ldx $ff,y \n" // read from $fe

                :"=x"(tmp)
                :
                :"a","y","p"
            );
            if(tmp != 0x5a){
                err_code = TEST_FAIL+3;
                break;
            }

            break;
        }

        // Test: Addressing Mode Wraparound: Indirect
        case 5: {
            RAM_BANK = 1;

            // Test 1: does JMP (indirect) move the PC to the correct location?
            //POKE(0x500, 0x00);
            //POKE(0x501, 0xa4); // write the address ($a400)
            //POKE(0xa400, 0x60); // write RTS to $a400
            __attribute__((leaf)) __asm__ volatile (
                "lda #<indirect_wrap_pass \n"
                "ldx #>indirect_wrap_pass \n"
                "sta $07fe \n"
                "stx $07ff \n"

                "ldx #0 \n"

                "jmp ($7fe) \n" 
                "inx \n"

                "indirect_wrap_pass: \n"

                :"=x"(tmp)
                :
                :"a","y","p"
            );
            if (tmp != 0) {err_code = TEST_FAIL+1; break;}


            
            // Test 2: if the indirect jump is at a page boundary,
            // the high byte gets read out of the same page.
            POKE(0xa0ff, 0xf0);
            POKE(0xa000, 0xbe); // $bef0 (success)

            POKE(0xbef0, 0x60); // write RTS to $bef0

            // if high byte is read out of the next page,
            // jump to $bff0
            POKE(0xa100, 0xbf); // $bff0 (fail)

            POKE(0xbff0, 0xE8); // write INX to $bff0
            POKE(0xbff1, 0x60); // write RTS to $bff1

            __attribute__((leaf)) __asm__ volatile (
                "ldx #0 \n"
                "ldy #0 \n"

                "jsr indirect_wrap2_here \n" // program counter returns here

                "indirect_wrap2_here: \n"
                "cpy #1 \n"
                "beq indirect_wrap2_pass \n"
                "iny \n"
                "jmp ($a0ff) \n"

                "indirect_wrap2_pass: \n"

                :"=x"(tmp)
                :
                :"a","y","p"
            );
            if (tmp == 0) {err_code = TEST_FAIL+2; break;}
            break;
        }

        // Test: Addressing Mode Wraparound: Indirect, X
        case 6: {

            // Test 1: does LDA (indirect, X) read from the expected address?
            POKE(0xfe, 0xfd);
            POKE(0xff, 0x00); // write the address ($00fd)
            POKE(0xfd, 0x5a); // write $5a to $00fd
            __attribute__((leaf)) __asm__ volatile (
                "ldx #$0e \n"
                "lda ($00f0, x)"

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if (tmp != 0x5a) {err_code = TEST_FAIL+1; break;}

            // Test 2: LDA (indirect, X) should be confined to the zero page.
            __attribute__((leaf)) __asm__ volatile (
                "ldx #$ff \n"
                "lda ($00ff, x)"

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if (tmp != 0x5a) {err_code = TEST_FAIL+2; break;}


            // Test 3: address bus should wrap around the page when reading
            // low and high bytes with indirect addressing.
            POKE(0xff, 0xfd);
            RAM_BANK = 0; // hijacking the RAM bank for a sec.
            __attribute__((leaf)) __asm__ volatile (
                "ldx #$00 \n"
                "lda ($00ff, x)"

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if (tmp != 0x5a) {err_code = TEST_FAIL+3; break;}


            break;
        }

        // Test: Addressing Mode Wraparound: Indirect, Y
        case 7: {
            // Test 1: does LDA (indirect),Y read from the expected address?
            // point to $07f0. Y will offset this
            POKE(0xfe, 0xf0); POKE(0xff, 0x07); 
            POKE(0x07fe, 0x5a); // $07fe will contain $5a
            __attribute__((leaf)) __asm__ volatile(
                "ldy #$0e \n"
                "lda ($00fe),y" // read from $07f0,y ($07ff)

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if(tmp != 0x5a){err_code = TEST_FAIL+1; break;}

            // Test 2: The Y indexing is allowed to cross page boundaries.
            POKE(0xfe, 0xf0); POKE(0xff, 0xa0); // point to $a0f0
            POKE(0xa110, 0x5a); // $a110 will contain $5a
            __attribute__((leaf)) __asm__ volatile(
                "ldy #$20 \n"
                "lda ($00fe),y" // read from $a0f0,y ($a110)

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if(tmp != 0x5a){err_code = TEST_FAIL+2; break;}


            // Test 3: The address bus wraps on a page boundary when 
            // reading the low and high bytes with indirect addressing.
            POKE(0xff, 0xf0);
            RAM_BANK = 0x07; // hijacking the RAM bank register for this test.
            //POKE(0x07fe, 0x5a); // $07fe will contain $5a
            __attribute__((leaf)) __asm__ volatile(
                "ldy #$0e \n"
                "lda ($00ff),y" // read from $07f0,y ($07fe)

                :"=a"(tmp)
                :
                :"x","y","p"
            );
            if(tmp != 0x5a){err_code = TEST_FAIL+3; break;}
            break;
        }

        // Test: (65c02): Open Bus
        // Read from Open Bus and see if it is correctly emulated.
        case 8: {
            // Test 1: Reading from open bus always returns
            // the high byte of the address read.

            // Read from YM2151 address port.
            // This *should* return open bus.
            // TODO: LOOP THROUGH I/O RANGE INSTEAD
            if(PEEK(0x9f40) != 0x9f) {
                if(PEEK(0x9f40) == 0) { // OPM2151/65C816 behavior.
                    err_code = 0; // pass the test, i guess?
                    cbm_k_plot(1,22);
                    printf("OPM2151/65C816 installed;");
                    cbm_k_plot(1,23);
                    printf("Open Bus is not testable");
                }
                else err_code = TEST_FAIL;
                
                break;
            }
            

            // Test 2: Indexed addressing crossing a page
            // boundary does not update the data bus.
            __attribute__((leaf)) __asm__ volatile (
                "ldx #$50 \n"
                "lda $9ef0,x \n"
                : "=a"(tmp)
                :
                : "x","y","p"
            );
            if(tmp != 0x9e) {
                err_code = TEST_FAIL+1;
                //printf("open bus result: %2x", tmp);
                break;
            }
            
            break;
        }

        // Test: (65c02): All NOPs
        case 9: {
            unsigned char tmp2;
            cbm_k_plot(1,22);
            // Pre-Test: are we on 65816?
            if(is_65816) {
                err_code = 0;
                printf("65C816 installed;");
                cbm_k_plot(1,23);
                printf("Most NOPs are not testable");
                break;
            }
            
            // Test 1: all 1 byte, 1 cycle NOPs
            wait_for_line_256();
            __attribute__((leaf)) __asm__ volatile(
                "ldy #$00 \n"
                "nop_onebyte_onecycle_run_the_test: \n"
                ".byte $03, $13, $23, $33, $43, $53, $63, $73 \n"
                ".byte $83, $93, $a3, $b3, $c3, $d3, $e3, $f3 \n"
                ".byte $0b, $1b, $2b, $3b, $4b, $5b, $6b, $7b \n"
                ".byte $8b, $9b, $ab, $bb,           $eb, $fb \n"
                "iny \n"
                "lda $9f26 \n"
                "and #$40 \n"
                "beq nop_onebyte_onecycle_run_the_test \n"
                :"=y"(tmp)
                :
                :"a","x","p"
            );
            if(tmp != 0x7) {err_code = TEST_FAIL+1; }
            printf("%3x",tmp);

            // Test 2: all 2 byte, 2 cycle NOPs
            wait_for_line_256();
            __attribute__((leaf)) __asm__ volatile(
                "ldx #$00 \n"
                "ldy #$00 \n"
                "nop_twobyte_twocycle_run_the_test: \n"
                "nop \n nop \n" // added for timing
                ".byte $02, $e8 \n" // INX if operand count is wrong
                ".byte $22, $ea \n"
                ".byte $42, $ea \n"
                ".byte $62, $ea \n"
                ".byte $82, $ea \n"
                ".byte $c2, $ea \n"
                ".byte $e2, $ea \n"
                "iny \n"
                "lda $9f26 \n"
                "and #$40 \n"
                "beq nop_twobyte_twocycle_run_the_test \n"
                :"=y"(tmp),"=x"(tmp2)
                :
                :"a","p"
            );
            if(tmp2) {err_code = TEST_FAIL+2; break;}
            if(tmp != 0x9) {err_code = TEST_FAIL+3; }
            printf("%3x",tmp);

            // Test 3: the ONE SINGULAR 2 byte, 3 cycle NOP
            wait_for_line_256();
            __attribute__((leaf)) __asm__ volatile(
                "ldx #$00 \n"
                "ldy #$00 \n"
                "nop_twobyte_threecycle_run_the_test: \n"
                ".byte $44, $e8 \n" // INX if operand count is wrong
                "iny \n"
                "lda $9f26 \n"
                "and #$40 \n"
                "beq nop_twobyte_threecycle_run_the_test \n"
                :"=y"(tmp),"=x"(tmp2)
                :
                :"a","p"
            );
            if(tmp2) {err_code = TEST_FAIL+4; break;}
            if(tmp != 0x12) {err_code = TEST_FAIL+5; }
            printf("%3x",tmp);

            // Test 4: all 2 byte, 4 cycle NOPs
            wait_for_line_256();
            __attribute__((leaf)) __asm__ volatile(
                "ldx #$00 \n"
                "ldy #$00 \n"
                "nop_twobyte_fourcycle_run_the_test: \n"
                ".byte $54, $e8 \n" // INX if operand count is wrong
                ".byte $f4, $ea \n"
                "iny \n"
                "lda $9f26 \n"
                "and #$40 \n"
                "beq nop_twobyte_fourcycle_run_the_test \n"
                :"=y"(tmp),"=x"(tmp2)
                :
                :"a","p"
            );
            if(tmp2) {err_code = TEST_FAIL+6; break;}
            if(tmp != 0xe) {err_code = TEST_FAIL+7; }
            printf("%3x",tmp);

            // Test 5: all 3 byte, 4 cycle NOPs
            wait_for_line_256();
            __attribute__((leaf)) __asm__ volatile(
                "ldx #$00 \n"
                "ldy #$00 \n"
                "nop_threebyte_fourcycle_run_the_test: \n"
                ".byte $dc, $ea, $e8 \n" // INX if operand count is wrong
                ".byte $fc, $ea, $ea \n"
                "iny \n"
                "lda $9f26 \n"
                "and #$40 \n"
                "beq nop_threebyte_fourcycle_run_the_test \n"
                :"=y"(tmp), "=x"(tmp2)
                :
                :"a","p"
            );
            if(tmp2) {err_code = TEST_FAIL+8; break;}
            if(tmp != 0xe) {err_code = TEST_FAIL+9; }
            printf("%3x",tmp);

            // Test 6: the ONE SINGULAR 3 byte, 8 cycle NOP
            wait_for_line_256();
            __attribute__((leaf)) __asm__ volatile(
                "ldy #$00 \n"
                "nop_threebyte_eightcycle_run_the_test: \n"
                ".byte $5c, $ea, $e8 \n" // INX if operand count is wrong
                "iny \n"
                "lda $9f26 \n"
                "and #$40 \n"
                "beq nop_threebyte_eightcycle_run_the_test \n"
                :"=y"(tmp),"=x"(tmp2)
                :
                :"a","p"
            );
            if(tmp2) {err_code = TEST_FAIL+10; break;}
            if(tmp != 0xe) {err_code = TEST_FAIL+11; }
            printf("%3x",tmp);

            break;
        }

        // Test: IO5-IO7 slow down the CPU
        case 0x0a: {
            unsigned char tmp2, tmp3, tmp4;

            #define io34result ((unsigned short)((tmp2<<8)+tmp))
            #define io56result ((unsigned short)((tmp4<<8)+tmp3))
            // Test 1: Reading from IO5-7 reduces CPU speed to 2mhz briefly.
            // first test IO3/4
            wait_for_line_256();
            __attribute__((leaf)) __asm__ volatile(
                "ldx #$00 \n"
                "ldy #$00 \n"

                "io34_speed_run_the_test: \n"
                "lda $9f60 \n" // IO3
                "lda $9f80 \n" // IO4
                "iny \n"
                "tya \n"
                "bne $1 \n"

                "inx \n" // this gets skipped if Y > 0

                "lda $9f26 \n"
                "and #$40 \n"
                "beq io34_speed_run_the_test \n"
                :"=y"(tmp),"=x"(tmp2)
                :
                :"a","p"
            );

            // then test IO5/6
            wait_for_line_256();
            __attribute__((leaf)) __asm__ volatile(
                "ldx #$00 \n"
                "ldy #$00 \n"

                "io56_speed_run_the_test: \n"
                "lda $9fa0 \n" // IO5
                "lda $9fc0 \n" // IO6
                "iny \n"
                "tya \n"
                "bne $1 \n"

                "inx \n" // this gets skipped if Y > 0

                "lda $9f26 \n"
                "and #$40 \n"
                "beq io56_speed_run_the_test \n"
                :"=y"(tmp3),"=x"(tmp4)
                :
                :"a","p"
            );

            // if it took the same amount of time
            // or less to read IO5/6, fail the test.
            if (io34result <= io56result) { 
                err_code = TEST_FAIL;
            }

            cbm_k_plot(20,22);
            printf("I03/4:%2x",tmp2); // low by
            cbm_k_plot(20,23);
            printf("I05/6:%4x",((tmp4<<8)+tmp3));

            break;

            #undef io34result
            #undef io56result
        }



        // Test: VERA Registers act as VRAM on read
        case 0x10: {
            // Test 1: does the APU have VRAM backing it?
            VERA.address_hi = 1;
            VERA.address = 0xf9c0;
            VERA.data0 = 0x5a; // write to $1:f9c0
            if(VERA.data0 != 0x5a) {
                // if the VERA data bus is NOT $5a, VRAM 
                // is not present in the PSG on read.
                err_code = TEST_FAIL+1;
                break;
            }

            // Test 2: does Color RAM have VRAM backing it?
            VERA.address_hi = 1|VERA_INC_1;
            VERA.address = 0xfa00;
            VERA.data0 = 0x5a; // write to $1:fa00
            VERA.address = 0xfa00;
            if(VERA.data0 != 0x5a) {
                // if the VERA data bus is NOT $5a, VRAM 
                // is not present in Color RAM on read.
                err_code = TEST_FAIL+2;
                VERA.address = 0xfa00;
                VERA.data0 = 0x00;
                break;
            }
            VERA.address = 0xfa00;
            VERA.data0 = 0x00;

            // Test 3: does Sprite RAM have VRAM backing it?
            VERA.address_hi = 1;
            VERA.address = 0xfc00;
            VERA.data0 = 0x5a; // write to $1:fa00
            VERA.address = 0xfc00;
            if(VERA.data0 != 0x5a) {
                // if the VERA data bus is NOT $5a, VRAM 
                // is not present in Sprite RAM on read.
                err_code = TEST_FAIL+2;
                break;
            }

            break;
        }


        // Test: VERA DCSEL behavior
        case 0x11: {
            // Test 1: unused slots in VERA DCSEL 
            // should return the version number.
            VERA.control = (40 << 1);
            
            if(PEEK(0x9f29) != 'V'){err_code = TEST_FAIL;}
            cbm_k_plot(1,22);
            printf("%c%d.%d.%d",PEEK(0x9f29),PEEK(0x9f2a),PEEK(0x9f2b),PEEK(0x9f2c));
            break;
        }



        default: {
            err_code = 0x00;
            break;
        }
    }
    //if(err_code >= 1){
        cbm_k_plot(28,22);
        printf("%2x", tmp);
    //} else {
    //    cbm_k_plot(0,22);
    //    printf("%64s", " ");
    //}
    return err_code; // return the error code.
    // if you didn't modify this value, the test passed.
}