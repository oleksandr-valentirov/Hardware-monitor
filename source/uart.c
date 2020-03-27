/*
 * File:   uart.c
 * Author: Aleksandr Valentirov
 *
 * Created on October 17, 2019, 10:26 AM
 */
#include <string.h>
#include <xc.h>

/*
 * universal function for sending 1 byte of data
 * takes address of byte and copies it to TXREG
 */
void uart_send_byte(void* num)
{
  unsigned char* ptr = (unsigned char*)num;
  for(size_t size = sizeof(*num); size > 0; size--)
  {
    while(TRMT == 0){}
    memcpy(&TXREG, ptr++, 1);
  }
  return;
}

/*
 * uses previous function to transfer array of characters or string
 * in future will be adopted for working with different types of data
 */
void uart_send_ascii(char* data, size_t data_size)
{
    for(size_t i = 0; i < data_size; i++)
    {
        uart_send_byte(&data[i]);
    }
    return;
}

// simply receive one byte and puts it to the given address
void uart_receive_byte(unsigned char* cell)
{
    *cell = RCREG;
    return;
}


/*
 * suppose that every system can have her own sizes of types
 * this function works with INT type
 * it takes address
 * then receives data byte by byte and puts it to this address
 */
void uart_receive_int(int* num)
{
    unsigned char* ptr = (unsigned char*)num;
    for(size_t i = sizeof(int); i > 0; i--)
    {
        *(ptr++) = RCREG;
    }
    return;
}


/*
 * suppose that every system can have her own sizes of types
 * this function works with FLOAT type
 * it takes address
 * then receives data byte by byte and puts it to this address
 */
void uart_receive_float(float* num)
{
    unsigned char* ptr = (unsigned char*)num;
    for(size_t i = sizeof(float); i > 0; i--)
    {
        *(ptr++) = RCREG;
    }
    return;
}
