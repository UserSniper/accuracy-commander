__attribute__((retain)) 
    #include "include.h"

void clear_msg(){
    cbm_k_plot(0,22);
    printf("%64s", " ");
}

void draw_title(){
    cbm_k_plot(7,2);
    printf("Accuracy Commander");
}

void run_every_test(){
    unsigned char i,j=0,k=0,tmp;
            
    for(i=0;i<(sizeof(test_names)>>1);i++){
        waitvsync();
        putchar(0x93);
        cbm_k_plot(6,11);
        printf("Running Test %3d/%3d",i,(sizeof(test_names)>>1));
        test_result[i] = run_test(i);
        if(test_result[i]) {k++;}
    }
    putchar(0x93);

    i=0;

    cbm_k_plot(12,6);
    printf("Results:");

    putchar(0x01);
    while (i < ((sizeof(test_names)>>1))) {
        if((i & 0x07) == 0){
            cbm_k_plot(8,8+(i>>3));
        }

        tmp = test_result[i];
        putchar(0x01);
        if(tmp < 0x80){
            switch(tmp){
                case 0: putchar(0x05); break;
                case 1: {
                    putchar(0x99); 
                    j++;
                    break;
                }
                default: {
                    putchar(0x96); 
                    break;
                }
            }
        } else {
            putchar(0x99);
            j++;
        }
        putchar(0x01);
        if(test_result[i] >= 0x80) {
            printf("%2x",(test_result[i]-0x80));
        }
        else if(test_result[i] >= 2) {
            printf("%2x",(test_result[i]-2));
        }
        else if(test_result[i]==0) printf("\x01 \x01");
        else printf("[]");
                
        i++;
    }
    putchar(0x01);
    
    cbm_k_plot(4,16);
    printf("Tests Passed:  %3d/%3d",j,k);

    do {
        waitvsync();
        poll_controller();
    } while (!pad_start_new);
}

void main(){
    cx16_k_screen_mode_set(11);

    draw_title();

    // zero out the test result memory.
    cx16_k_memory_fill((void*)0x400,0x400,0x00);

    draw_test_names(0);
    draw_test_select(0);
    while(1){
        waitvsync();
        poll_controller();
        
        if(selection < scroll) {
            scroll--;
            draw_test_names(scroll);
        }
        if((selection - scroll) >= 16) {
            scroll++;
            draw_test_names(scroll);
        }
        draw_test_select(scroll);




        if(pad_down_new) selection++;
        if(pad_up_new) selection--;
        if(selection == 0xff) selection = 0;
        if(selection >= (sizeof(test_names)>>1)) selection--;




        if(pad_a_new){
            clear_msg();
            cbm_k_plot(1, 4+(selection-scroll));
            printf("\x01....\x01");
            waitvsync();
            test_result[selection] = run_test(selection);
        }

        if(pad_start_new){
            run_every_test();
            draw_title();
            draw_test_names(scroll);
        }
    }
    

    return;
}