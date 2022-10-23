/* Wirepas Oy licensed under Apache License, Version 2.0
 *
 * See file LICENSE for full license details.
 *
 */
#include "serial.h"

#include <errno.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/serial.h>

#include "serial_termios2.h"

#define LOG_MODULE_NAME "SERIAL"
#define MAX_LOG_LEVEL INFO_LOG_LEVEL
#include "logger.h"

static int fd = -1;

/** \brief Forward declaration of internal open */
static int int_open();

/** \brief  Port name to open */
static char m_port_name[256];

/** \brief Bitrate to use */
static unsigned long m_bitrate;

static int set_interface_attribs(int fd, unsigned long bitrate, int parity)
{
    struct termios tty;
    struct serial_struct serial_s;

    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0)
    {
        LOGE("Error %d from tcgetattr", errno);
        return -1;
    }

    // default to 9600 bps, but use TCSETS2 to set the actual bitrate
    // use a bitrate that is not 115200 or 125000 bps here, to make sure
    // TCSETS2 actually sets the bitrate
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    // 8-bit chars
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    // disable break processing
    tty.c_iflag &= ~IGNBRK;
    // disable CR -> NL translation
    tty.c_iflag &= ~ICRNL;
    // no signaling chars, no echo, no canonical processing
    tty.c_lflag = 0;
    // no remapping, no delays
    tty.c_oflag = 0;
    // VMIN=0, VTIME=1 => blocking for max 100ms
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    // shut off xon/xoff ctrl
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // ignore modem controls, enable reading
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        LOGE("Error %d from tcsetattr", errno);
        return -1;
    }

    if (serial_set_termios2_bitrate(fd, bitrate) != 0) {
        return -1;
    }

    // Set low latency flag to serial
    ioctl(fd, TIOCGSERIAL, &serial_s);
    serial_s.flags |= ASYNC_LOW_LATENCY;
    ioctl(fd, TIOCSSERIAL, &serial_s);

    return 0;
}

static int int_open(serial_t *handle)
{
    int fd = open(handle->port_name, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        LOGE("Error %d opening serial link %s: %s\n",
	     errno, handle->port_name, strerror(errno));
        return -1;
    }

    // set the requested bitrate, 8n1, no parity
    if (set_interface_attribs(fd, handle->bitrate, 0) < 0) {
        close(fd);
        handle->fd = -1;
        return -1;
    }

    handle->fd = fd;

    LOGD("Serial opened\n");
    return 0;
}

/****************************************************************************/
/*                Public method implementation                              */
/****************************************************************************/
serial_t* serial_open(const char * port_name, unsigned long bitrate)
{
    serial_t *handle = calloc(1, sizeof(*handle));

    if (!handle) {
	LOGE("memory allocation error\n");
	return NULL;
    }

    // Copy the settings locally
    strcpy(handle->port_name, port_name);
    handle->bitrate = bitrate;

    if (int_open(handle) < 0) {
	free(handle);
	return NULL;
    }

    return handle;
}

int serial_close(serial_t *handle)
{
    if (handle->fd < 0) {
        LOGW("Link already closed\n");
        return -1;
    }

    if (close(handle->fd) < 0) {
        LOGW("Error %d closing serial link: %s\n", errno, strerror(errno));
        return -1;
    }

    handle->fd = -1;
    LOGD("Serial closed\n");
    return 0;
}

static ssize_t get_single_char(int fd, unsigned char * c, unsigned int timeout_ms)
{
    static unsigned char m_buffer[128];
    static uint8_t m_elem = 0;
    static uint8_t m_read = 0;
    ssize_t read_bytes;
    uint16_t attempts;

    // Do we still have char buffered?
    if (m_elem > 0)
    {
        *c = m_buffer[m_read++];
        m_elem--;
        return 1;
    }

    // Convert timeout_ms in attempt of 100ms (timeout set)
    attempts = ((timeout_ms + 99) / 100) - 1;

    // Local buffer is empty, refill it
    do
    {
        read_bytes = read(fd, m_buffer, sizeof(m_buffer));
        if (read_bytes > 0)
        {
            m_elem = read_bytes;
            m_read = 0;
            *c = m_buffer[m_read++];
            m_elem--;
            return 1;
        }
    } while (attempts-- > 0);

    LOGD("Timeout to wait for char on serial line\n");
    return 0;
}

int serial_read(serial_t * h, unsigned char * c, unsigned int timeout_ms)
{
    if (h->fd < 0)
    {
        LOGE("No serial link opened\n");
        return -1;
    }

    return get_single_char(h->fd, c, timeout_ms);
}

int serial_write(serial_t * h, const unsigned char * buffer, unsigned int buffer_size)
{
    if (h->fd < 0)
    {
        LOGE("No serial link opened\n");
        // Try to reopen
        if (int_open(h) < 0)
        {
            // Wait a bit before next try
            usleep(1000 * 1000);
            return 0;
        }
        LOGI("Serial reopened\n");
    }

    ssize_t ret = write(h->fd, buffer, buffer_size);
    if (ret < 0)
    {
        LOGE("Error in write: %d\n", errno);
        LOGE("Close connection\n");
        serial_close(h);

        // Connection will be checked reopen at next try

        return 0;
    }
    return ret;
}
