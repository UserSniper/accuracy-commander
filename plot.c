void cbm_k_plot(unsigned char x, unsigned char y){
    __attribute__((leaf)) __asm__ volatile (
        "clc \n"
        "jsr $fff0 \n"
        : // output.
        : "x"(y), "y"(x) // input. confusing, right fellas?
        : "a", "c", "p" // clobbers/leftovers. unused A/X/Y/C/P must be included here
    );
}