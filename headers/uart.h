/*
 * File:   uart.h
 * Author: aleksandr
 *
 * Created on October 17, 2019, 10:28 AM
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif

void uart_send_byte(void* num);
void uart_send_ascii(char* data, size_t data_size);
void uart_receive_byte(unsigned char* cell);
void uart_receive_int(int* num);
void uart_receive_float(float* num);


#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */
