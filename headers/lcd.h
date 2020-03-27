/* 
 * File:   lcd.h
 * Author: Aleksandr Valentirov
 *
 * Created on March 8, 2020, 1:51 PM
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

void lcd_send_data(unsigned char data);
void lcd_send_cmd(unsigned char cmd);
void lcd_init(void);
void lcd_set_cursor(unsigned char r, unsigned char c);
void lcd_write_ascii(unsigned char *str, size_t data_size);
void lcd_write_string(unsigned char *str);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

