#ifndef SERIAL_TERMIOS2_H
#define SERIAL_TERMIOS2_H

/**
 * \file    serial_termios2.h
 *          A function to set a custom bitrate on Linux, using struct termios2.
 *          This needs to be in its own file due to Linux header file issues.
 */

int serial_set_termios2_bitrate(int fd, unsigned long bitrate);

#endif /* SERIAL_TERMIOS2_H */
