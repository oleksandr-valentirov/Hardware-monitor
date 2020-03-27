/*
 * File:   main.c
 * Author: Aleksandr Valentirov
 *
 * Created on March 21, 2020, 7:38 PM
 */


#include <xc.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <pic16f18875.h>
#include "config.h"
#include "lcd.h"
#include "uart.h"

#define _XTAL_FREQ 4000000
#define _DESIRED_BAUD_RATE 9600
#define _BAUD_RATE (short)(_XTAL_FREQ /_DESIRED_BAUD_RATE/16 - 1)
#define _DESIRED_TIME_SEC 3
#define _T0_START (short)(65536 - _DESIRED_TIME_SEC * _XTAL_FREQ / (4 * 64))

char raw[100];  // raw data from UART
char raw_ptr = 0;  // position pointer
char result[20][4];  // parsed data for LCD
bool data_update = false;  // udpate LCD
bool scr_update = true;  // update screen
bool no_connection = false;  // update sceen if connection was lost
bool update_status = false;  // switcher to update screen on connection lost only once
unsigned char lcd_mode = 0;  // used in 16x2 and maybe later in 20x4 to update views

// functions, used in main() or interrupt routine
void T0_init(void);
void EUSART_init(void);
void parse_data(void);
void lcd_bar_characters(void);

// scr - SCREEN, not fucking SOURCE (src)
void scr_16x2(void);
void scr_20x4(void);

// screen selection
//void (*scr_mode)(void) = &scr_16x2;
void (*scr_mode)(void) = &scr_20x4;

void main(void) {
    OSCCON1bits.NOSC = 0b110;  // clock source
    OSCCON1bits.NDIV = 0;  // frequency divider
    OSCFRQbits.HFFRQ = 0b010;  // 3 lsbs set clock frequency
    
    // init all needed modules
    lcd_init();
    lcd_bar_characters();
    EUSART_init();
    T0_init();
    
    // global and peripheral interrupts
    GIE = 1;
    PEIE = 1;
    
    T0EN = 1;  // Timer0 module start
    
    // debug
    TRISDbits.TRISD1 = 0;
    LATDbits.LATD1 = 0;
    TRISDbits.TRISD0 = 0;
    LATDbits.LATD0 = 0;
    
    while(1){
        // update screen if new data was received
        if (data_update){
            LATDbits.LATD1 = 1;
            parse_data();
            (*scr_mode)();
            data_update = false;
            LATDbits.LATD1 = 0;
            
            TMR0H = _T0_START >> 8;
            TMR0L = 0x00FF & _T0_START;
            if(!T0EN){
                T0EN = 1;
            }
        }
        
        // update screen if connection was lost
        if(no_connection && update_status){
            lcd_send_cmd(1);  // clear screen
            lcd_send_cmd(0x80);  // "Fucking Surprise" from LCD hardware
            update_status = false;
            scr_update = true;  // update UI if connection restored
            if(scr_mode == &scr_20x4){
                lcd_set_cursor(2, 10);
                lcd_write_string("no\0");
                lcd_set_cursor(3, 6);
                lcd_write_string("connection\0");
            }
            else{
                lcd_set_cursor(1, 8);
                lcd_write_string("no\0");
                lcd_set_cursor(2, 3);
                lcd_write_string("connection\0");
            }
        }
    }
    return;
}


void __interrupt() ISR(void){
    // UART interrupt on receive
    if (RCIE & RCIF){      
        no_connection = false;
        char tmp = RCREG;
        raw[raw_ptr] = tmp;
        raw_ptr++;
        if (tmp == 'E'){
            data_update = true;
            raw_ptr = 0;
        }        
    }
    // Timer0 interrupt on overflow
    if (TMR0IE & TMR0IF){
        T0EN = 0;
        TMR0IF = 0;
        LATDbits.LATD0 = ~LATDbits.LATD0;
        no_connection = true;
        update_status = true;
    }
}


// Timer0 initialize
void T0_init(void){
    T016BIT = 1;  // 16-bit mode
    T0CON1bits.T0CS = 2;  // Fosc/4 as clock source
    T0CON1bits.T0CKPS = 6;  // pre-scaler = 64
    // pre-loading timer values for more accurate measurement
    TMR0H =  _T0_START >> 8;
    TMR0L = 0x00FF & _T0_START;
    // enabling interrupts
    TMR0IE = 1;
    TMR0IF = 0;
}


// EUSART (UART) initialize
void EUSART_init(void){
    // mode
    TX1STAbits.BRGH = 1;  // HIGH-SPEED mode
    TX1STAbits.SYNC = 0;  // ASYNC mode
    SP1BRG = _BAUD_RATE;
    // workflow
    RC1STAbits.SPEN = 1;  // enable serial port
    RC1STAbits.CREN = 1;  // ENABLE continuous receive
    // pins
    ANSELCbits.ANSC7 = 0;
    TRISC7 = 1;
    // interrupt on receive
    RCIE = 1;
    return;
}


// this shit parses data from UART input
// I think, it can be better
// I like it works like Python`s split() function
void parse_data(void){
//    char* str_1 = "54;43;0;1;44;100E";  // dummy
    memset(result, '\0', sizeof(result));
    
    short coma_p = 0;
    short coma_n = strchr(raw, ';') - raw;
    short diff = coma_n - coma_p;
    unsigned char coma_counter = 0;
    
    while(diff > 0){
        char i_str = 0;
        memcpy(result[coma_counter], raw + coma_p, diff);
        coma_counter++;

        coma_p = coma_n + 1;
        coma_n = strchr(raw + coma_p + 1, ';') - raw;
        diff = coma_n - coma_p;
    }
    
    // piece after last semicolon
    short e = strchr(raw, 'E') - raw;
    memcpy(result[coma_counter], raw + coma_p, e - coma_p);
}


// takes a position and clears N cells after it, including it
void clear_n_cells(unsigned char row, unsigned char pos, unsigned char n){
    lcd_set_cursor(row, pos);
    while(n){
        lcd_send_data(' ');
        n--;
    }
    return;
}


// draws progress bars with custom characters
void draw_bar_chart(unsigned char value, unsigned int max_cells){
    int v_cells = (value*max_cells)/100;
    for(int i = 0; i < max_cells; i++, v_cells--){
        // the most left character
        if (!i){
            if (v_cells == 0)
                lcd_send_data(0x00);  // blank
            else if(v_cells > 0)
                lcd_send_data(0x03);  // full
        }
        // middle characters
        else if(max_cells - 1 > i > 0){
            if(v_cells <= 0)
                lcd_send_data(0x01);  // blank
            else
                lcd_send_data(0xFF);  // full
        }
        // the most right character
        else if(i == (max_cells - 1)){
            if (v_cells != 1)
                lcd_send_data(0x02);  // blank
            else
                lcd_send_data(0x04);  // full
        }
    }
}


// serves first view mode of 16x2
// CPU temp, load and RAM load
void update_0_16x2(void){
    // CPU tmp
    clear_n_cells(1, 5, 12);
    lcd_set_cursor(1, 5);
    lcd_write_string(result[0]);
    lcd_send_data(0b11011111);  // degree sign
    
    // CPU load
    lcd_set_cursor(1, 8);
    // ToDo - change second arg to 4 if your max temp is bigger then 99°C
    draw_bar_chart(atoi(result[4]), 5);
    lcd_set_cursor(1, 13);
    lcd_write_string(result[4]);
    lcd_send_data('%');
    
    
    // RAM
    clear_n_cells(2, 5, 12);
    lcd_set_cursor(2, 5);
    lcd_write_string(result[6]);
    lcd_send_data('%');
    lcd_set_cursor(2, 9);
    draw_bar_chart(atoi(result[6]), 8);
}


// serves first and only one so far view for 20x4 LCD
// not because it is hard to create one more
// but because it is enough for me now
void update_20x4(void){
    // CPU tmp
    clear_n_cells(1, 5, 16);
    lcd_set_cursor(1, 5);
    lcd_write_string(result[0]);
    lcd_send_data(0b11011111);  // degree sign
    
    // CPU load
    lcd_set_cursor(1, 8);
    draw_bar_chart(atoi(result[4]), 9);
    lcd_set_cursor(1, 17);
    lcd_write_string(result[4]);
    lcd_send_data('%');
    
    
    // GPU tmp
    clear_n_cells(2, 5, 16);
    lcd_set_cursor(2, 5);
    lcd_write_string(result[1]);
    lcd_send_data(0b11011111);  // degree sign
    
    // GPU load
    lcd_set_cursor(2, 8);
    draw_bar_chart(atoi(result[5]), 9);
    lcd_set_cursor(2, 17);
    lcd_write_string(result[5]);
    lcd_send_data('%');
    
    
    // VRAM load
    clear_n_cells(3, 6, 15);
    lcd_set_cursor(3, 6);
    lcd_write_string(result[7]);
    lcd_send_data('%');
    lcd_set_cursor(3, 11);
    draw_bar_chart(atoi(result[7]), 10);
    
    
    // SDRAM load
    clear_n_cells(4, 7, 14);
    lcd_set_cursor(4, 7);
    lcd_write_string(result[6]);
    lcd_send_data('%');
    lcd_set_cursor(4, 11);
    draw_bar_chart(atoi(result[6]), 10);
}


// serves 16x2 LCD workflow
// should have different modes for different data output views
// because it is small
void scr_16x2(void){
    // update screen of display change or connection restore
    if (scr_update){
        lcd_send_cmd(1);  // clear display
        switch(lcd_mode){
            case 0:
                lcd_set_cursor(1, 1);
                lcd_write_ascii("CPU:", 4);

                lcd_set_cursor(2, 1);
                lcd_write_ascii("RAM:", 4);
                break;
            case 1:
                break;
            finally:
                scr_update = false;
                break;
        }
    }
    // update data
    switch(lcd_mode){
        case 0:
            update_0_16x2();
            break;
        case 1:
            break;
    }
    return;
}


// serves 20x4 LCD workflow
// because there is only 1 mode, it is simpler than 16x2 LCD`s function
void scr_20x4(void){
    if (scr_update){
        lcd_send_cmd(1);  // clear display
        lcd_set_cursor(1, 1);
        lcd_write_ascii("CPU:", 4);

        lcd_set_cursor(2, 1);
        lcd_write_ascii("GPU:", 4);
                
        lcd_set_cursor(3, 1);
        lcd_write_ascii("VRAM:", 5);
                
        lcd_set_cursor(4, 1);
        lcd_write_ascii("SDRAM:", 6);
        scr_update = false;   
    }
    update_20x4();
    return;
}


// writes custom bar characters to the LCD`s memory
void lcd_bar_characters(void){
    // x3 - 0x08, 0x0F, 0x02, 0x1E
    // x6 - 0x00
    unsigned char rows[] = {0x03, 0x06, 0x08, 0x06, 0x03, 0x1F, 0x00, 0x1F, 0x18, 0x04, 0x02, 0x04, 0x18, 0x03, 0x07, 0x0F, 0x07, 0x03, 0x18, 0x1C, 0x1E, 0x1C, 0x18};
    
    lcd_send_cmd(0x40);
    for(char i = 0; i < 23; i++){
        if(rows[i] == 0x08 || rows[i] == 0x0F || rows[i] == 0x02 || rows[i] == 0x1E){
            for(char n = 0; n < 4; n++){
                lcd_send_data(rows[i]);
            }
        }
        else if (rows[i] == 0x00){
            for(char n = 0; n < 6; n++){
                lcd_send_data(rows[i]);
            }
        }
        else{
            lcd_send_data(rows[i]);
        }
    }
    lcd_set_cursor(1, 1);
}