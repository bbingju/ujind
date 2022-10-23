#ifndef SERIAL_H
#define SERIAL_H

/**
 * \file    serial.h
 *          Low level serial interface. Used for transmitting and
 *          receiving data via UART.
 */

typedef struct {
    char port_name[256];
    int fd;
    unsigned long bitrate;
} serial_t;


serial_t* serial_open(const char * port_name, unsigned long bitrate);
int serial_close(serial_t *);
int serial_read(serial_t * handle, unsigned char * c, unsigned int timeout_ms);
int serial_write(serial_t * handle, const unsigned char * buffer, unsigned int buffer_size);

#endif /* SERIAL_H */
