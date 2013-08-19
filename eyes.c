#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <stat.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/types.h>

#define MODEMDEVICE "/dev/face"
#define BAUDRATE 115200
#define TRUE 1
#define FALSE 0
#define _POSIX_SOURCE 1
#define STREAM_SIZE 10      /* How long the message is */

volatile int STOP = false;

void signal_handler_IO ( int status );
int wait_flag = TRUE;
char buffer[STREAM_SIZE];

/* Non-Canonical Input Processing */

char* get_eye_status( char eye_position )
{
    int file_des, res;
    struct termios oldtio, newtio;

    file_des = open( MODEMDEVICE, O_RDWR | O _NOCTTY );
    if( !file_des ) 
    {
        perror(MODEMDEVICE);
        exit(-1);
    }

    tcgetattr( file_des, &oldtio );

    bzero( &newtio, sizeof(newtio) );
    /* 
    *BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
    *CRTSCTS : output hardware flow control (only used if the cable has
    *all necessary lines. See sect. 7 of Serial-HOWTO)
    *CS8     : 8n1 (8bit,no parity,1 stopbit)
    *CLOCAL  : local connection, no modem contol
    *CREAD   : enable receiving characters
    *IGNPAR  : ignore bytes with parity errors.
    */
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* Input mode: ICANON = 0; for non canonical mode */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0;      /* Blocking on set number of chars */
    newtio.c_cc[VMIN]  = 10;     /* Block the read() call until 10 chars recieved */

    /* TCIFLUSH : Flush data recieved but not read
     * TCSANOW  : Change it immediately
     */
    tcflush( file_des, TCIFLUSH ); 
    tcsetattr( file_des, TCSANOW, &newtio );

    while ( STOP == FALSE )
    {
        res = read( file_des, buffer, STREAM_SIZE );
        buffer[res] = 0;        /* NULL termination */
        // printf(":%s:%d\n", buffer, res);
        if ( buffer[0] = 'X' )  /* 'X' is a stop bit, if hit, then stop */
            STOP = TRUE;
    }

    tcsetattr( file_des, TCSANOW, &oldtio );
    return buffer;
}
