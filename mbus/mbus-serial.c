#ifdef _WIN32
//window
#include <string.h>
#include <BaseTsd.h>
#else
//linux
#include <unistd.h>
#include <strings.h>
#include <string.h>
#endif

#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mbus-serial.h"
#include "mbus-protocol.h"

#define MBUS_ERROR(...) fprintf (stderr, __VA_ARGS__)
#ifdef DEBUG
#define MBUS_SERIAL_DEBUG
#endif

#define PACKET_BUFF_SIZE 512

#ifdef _WIN32
#ifndef SSIZE_MAX
#ifdef _WIN64
#define SSIZE_MAX _I64_MAX
#else
#define SSIZE_MAX LONG_MAX
#endif
#endif
#define __PRETTY_FUNCTION__       __FUNCTION__
#define O_NOCTTY 0x0000 // No idea if this makes sense
typedef SSIZE_T ssize_t;
#define read(...) readFromSerial(__VA_ARGS__)
#define write(...) writeToSerial(__VA_ARGS__)
#define select(...) selectSerial(__VA_ARGS__)
#define open(...) openSerial(__VA_ARGS__)
#define close(...) closeSerial(__VA_ARGS__)
#endif

int mbus_serial_connect(mbus_handle *handle)
{
    mbus_serial_data *serial_data;
    const char *device;
    struct termios *term;

    if (handle == NULL)
        return -1;

    serial_data = (mbus_serial_data *) handle->auxdata;
    if (serial_data == NULL || serial_data->device == NULL)
        return -1;

    device = serial_data->device;
    term = &(serial_data->t);

    // Use blocking read and handle it by serial port VMIN/VTIME setting
    if ((handle->fd = open(device, O_RDWR | O_NOCTTY)) < 0) {
        MBUS_ERROR("%s: failed to open tty.\n", __PRETTY_FUNCTION__);
        return -1;
    }

    memset(term, 0, sizeof(*term));
    term->c_cflag |= (CS8|CREAD|CLOCAL);
    term->c_cflag |= PARENB;

    // No received data still OK
    term->c_cc[VMIN] = (cc_t) 0;

    // Wait at most 0.2 sec.Note that it starts after first received byte!!
    // I.e. if CMIN>0 and there are no data we would still wait forever...
    //
    // The specification mentions link layer response timeout this way:
    // The time structure of various link layer communication types is described in EN60870-5-1. The answer time
    // between the end of a master send telegram and the beginning of the response telegram of the slave shall be
    // between 11 bit times and (330 bit times + 50ms).
    //
    // Nowadays the usage of USB to serial adapter is very common, which could
    // result in additional delay of 100 ms in worst case.
    //
    // For 2400Bd this means (330 + 11) / 2400 + 0.15 = 292 ms (added 11 bit periods to receive first byte).
    // I.e. timeout of 0.3s seems appropriate for 2400Bd.
    term->c_cc[VTIME] = (cc_t) 3; // Timeout in 1/10 sec

    cfsetispeed(term, B2400);
    cfsetospeed(term, B2400);
    tcsetattr(handle->fd, TCSANOW, term);
    return 0;
}

int mbus_serial_set_baudrate(mbus_handle *handle, long baudrate)
{
    speed_t speed;
    mbus_serial_data *serial_data;

    if (handle == NULL)
        return -1;

    serial_data = (mbus_serial_data *) handle->auxdata;
    if (serial_data == NULL)
        return -1;

    switch (baudrate) {
        case 300:
            speed = B300;
            serial_data->t.c_cc[VTIME] = (cc_t) 13; // Timeout in 1/10 sec
            break;
        case 600:
            speed = B600;
            serial_data->t.c_cc[VTIME] = (cc_t) 8;  // Timeout in 1/10 sec
            break;
        case 1200:
            speed = B1200;
            serial_data->t.c_cc[VTIME] = (cc_t) 5;  // Timeout in 1/10 sec
            break;
        case 2400:
            speed = B2400;
            serial_data->t.c_cc[VTIME] = (cc_t) 3;  // Timeout in 1/10 sec
            break;
        case 4800:
            speed = B4800;
            serial_data->t.c_cc[VTIME] = (cc_t) 3;  // Timeout in 1/10 sec
            break;
        case 9600:
            speed = B9600;
            serial_data->t.c_cc[VTIME] = (cc_t) 2;  // Timeout in 1/10 sec
            break;
        case 19200:
            speed = B19200;
            serial_data->t.c_cc[VTIME] = (cc_t) 2;  // Timeout in 1/10 sec
            break;
        case 38400:
            speed = B38400;
            serial_data->t.c_cc[VTIME] = (cc_t) 2;  // Timeout in 1/10 sec
            break;
       default:
            return -1; // unsupported baudrate
    }
    // Set input baud rate
    if (cfsetispeed(&(serial_data->t), speed) != 0) {
        return -1;
    }
    // Set output baud rate
    if (cfsetospeed(&(serial_data->t), speed) != 0) {
        return -1;
    }
    // Change baud rate immediately
    if (tcsetattr(handle->fd, TCSANOW, &(serial_data->t)) != 0) {
        return -1;
    }
    return 0;
}

int mbus_serial_disconnect(mbus_handle *handle)
{
    if (handle == NULL) {
        return -1;
    }

    if (handle->fd < 0) {
        return -1;
    }

    close(handle->fd);
    handle->fd = -1;
    return 0;
}

void mbus_serial_data_free(mbus_handle *handle)
{
    mbus_serial_data *serial_data;

    if (handle) {
        serial_data = (mbus_serial_data *) handle->auxdata;
        if (serial_data == NULL) {
            return;
        }
        free(serial_data->device);
        free(serial_data);
        handle->auxdata = NULL;
    }
}

int mbus_serial_send_frame(mbus_handle *handle, mbus_frame *frame)
{
    unsigned char buff[PACKET_BUFF_SIZE];
    int len, ret;

    if (handle == NULL || frame == NULL) {
        return -1;
    }

    #ifdef _WIN32
    if (GetFileType(getHandle()) != FILE_TYPE_CHAR )
    #else
    if (isatty(handle->fd) == 0)
    #endif
    {
        MBUS_ERROR("%s: connection not open\n", __PRETTY_FUNCTION__);
        return -1;
    }

    len = mbus_frame_pack(frame, buff, sizeof(buff));
    if (len <= 0) {
        MBUS_ERROR("%s: mbus_frame_pack failed\n", __PRETTY_FUNCTION__);
        return -1;
    }

    ret = write(handle->fd, buff, len);
    if (ret != len) {
        MBUS_ERROR("%s: Failed to write frame to socket (ret = %d: %s)\n", __PRETTY_FUNCTION__, ret, strerror(errno));
        return -1;
    }

    tcdrain(handle->fd);
    return 0;
}

int mbus_serial_recv_frame(mbus_handle *handle, mbus_frame *frame)
{
    unsigned char buff[PACKET_BUFF_SIZE];
    int timeouts;
    ssize_t len, nread;

    if (handle == NULL || frame == NULL) {
        MBUS_ERROR("%s: Invalid parameter.\n", __PRETTY_FUNCTION__);
        return MBUS_RECV_RESULT_ERROR;
    }

    #ifdef _WIN32
    if (GetFileType(getHandle()) != FILE_TYPE_CHAR )
    #else
    if (isatty(handle->fd) == 0)
    #endif
    {
        MBUS_ERROR("%s: Serial connection is not available.\n", __PRETTY_FUNCTION__);
        return MBUS_RECV_RESULT_ERROR;
    }

    memset((void *)buff, 0, sizeof(buff));

    len = 0;
    timeouts = 0;
    while (1) {
        if (len > PACKET_BUFF_SIZE) {
            return MBUS_RECV_RESULT_ERROR;
        }

        if (len == 0) {
            nread = read(handle->fd, &buff[0], 1);
        } else {
            nread = read(handle->fd, &buff[len], PACKET_BUFF_SIZE - len);
        }
        
        if (nread == -1) {
            return MBUS_RECV_RESULT_ERROR;
        } else if (nread == 0) {
            if (timeouts++ >= 3) {
                return MBUS_RECV_RESULT_TIMEOUT;
            }
        } else {
            timeouts = 0;
        }

        if (len == 0 && MBUS_FRAME_START != buff[0]) {
            continue;
        }

        len += nread;
        if (0 == mbus_parse(frame, buff, len)) {
            return MBUS_RECV_RESULT_OK;
        }
    };
}
