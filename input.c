unsigned short pad, pad_old, pad_new; // previous frame's input

#define pad_b       (low_byte(pad) & 0x80)
#define pad_y       (low_byte(pad) & 0x40)
#define pad_select  (low_byte(pad) & 0x20)
#define pad_start   (low_byte(pad) & 0x10)
#define pad_up      (low_byte(pad) & 0x08)
#define pad_down    (low_byte(pad) & 0x04)
#define pad_left    (low_byte(pad) & 0x02)
#define pad_right   (low_byte(pad) & 0x01)

#define pad_a       (high_byte(pad) & 0x80)
#define pad_x       (high_byte(pad) & 0x40)
#define pad_l       (high_byte(pad) & 0x20)
#define pad_r       (high_byte(pad) & 0x10)


#define pad_b_new       (low_byte(pad_new) & 0x80)
#define pad_y_new       (low_byte(pad_new) & 0x40)
#define pad_select_new  (low_byte(pad_new) & 0x20)
#define pad_start_new   (low_byte(pad_new) & 0x10)
#define pad_up_new      (low_byte(pad_new) & 0x08)
#define pad_down_new    (low_byte(pad_new) & 0x04)
#define pad_left_new    (low_byte(pad_new) & 0x02)
#define pad_right_new   (low_byte(pad_new) & 0x01)

#define pad_a_new       (high_byte(pad_new) & 0x80)
#define pad_x_new       (high_byte(pad_new) & 0x40)
#define pad_l_new       (high_byte(pad_new) & 0x20)
#define pad_r_new       (high_byte(pad_new) & 0x10)


#define high_byte(a) *((unsigned char*)&a+1)
#define low_byte(a) *((unsigned char*)&a)


struct __mouse {
    unsigned short x;
    unsigned short y;
    unsigned char b,s;
};
#define mousePosZP (*(volatile struct __mouse *)0xd8)
// see pad vars above

struct __zp mouse {
    unsigned short x;
    unsigned short y;
    unsigned char button;
      signed char scroll;

    unsigned char button_old;
    unsigned char button_new;
} mouse;

#define mouse_left (mouse.button & 0x01)
#define mouse_right (mouse.button & 0x02)
#define mouse_middle (mouse.button & 0x04)
#define mouse_button4 (mouse.button & 0x10)
#define mouse_button5 (mouse.button & 0x20)

#define mouse_left_new (mouse.button_new & 0x01)
#define mouse_right_new (mouse.button_new & 0x02)
#define mouse_middle_new (mouse.button_new & 0x04)
#define mouse_button4_new (mouse.button_new & 0x10)
#define mouse_button5_new (mouse.button_new & 0x20)

void poll_controller(){
    //unsigned char tmp1 = 0;

    mouse.button_old = mouse.button;

    //mouse.button = cx16_k_mouse_get(&mousePosZP);
    __attribute__((leaf)) __asm__ volatile(
        "ldx #$d8 \n"
        "jsr $ff6b \n"
        "sta $dc \n"
        "stx $dd \n"
    ); // call "mouse_get" kernal function*/

    mouse.x = mousePosZP.x;
    mouse.y = mousePosZP.y;
    mouse.button = mousePosZP.b;
    mouse.scroll = mousePosZP.s;
    //mouse = TMP;

    mouse.button_new = (mouse.button_old ^ mouse.button) & mouse.button;



    pad_old = pad;
    //pad = pad_new;// this line is solely here for the linker
    
    __asm__("lda #0"); // Check joystick 0 
    __asm__("jsr $FF56");  // Call "joystick_get" Kernal function
    
    // Get the status bytes from the A and X registers
    __asm__("eor #%11111111"); // negate low byte
    __asm__("sta $dc");
    __asm__("txa");
    __asm__("eor #%11111111"); // negate high byte
    __asm__("sta $dd");
    
    low_byte(pad) = mousePosZP.b;
    high_byte(pad) = mousePosZP.s;
    //if(mouse_left)low_byte(pad)^=0x80; // toggle B if left mouse held
    //if(mouse_left)low_byte(pad)^=0x80; // toggle B if left mouse held

    // pad_old: previous frame's inputs
    // pad:     current frame's inputs
    // pad_new: new inputs (first frame pressed)
    pad_new = (pad_old ^ pad) & pad;
}


unsigned char mouse_in_window(
    const unsigned short x,
    const unsigned short y,
    const unsigned char width,
    const unsigned char height){

    if(mouse.x < x) goto nope;
    if(mouse.y < y) goto nope;
    if(mouse.x < x+width){
        if(mouse.y < y+height){
            return 1;
        }
    }

    nope: 
    return 0;
}