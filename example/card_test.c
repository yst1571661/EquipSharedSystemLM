
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "serial.h"
#include "nuc_config.h"


#ifdef NUC951

#define DEBUG_ICDEV

#ifdef DEBUG_ICDEV
#define debug(format,args...) do {printf("\nline: %d	"format"\n", __LINE__, ##args);\
                                fflush(stdout); } while (0)
#else
#define debug(format,args...)
#endif


#define ICDEV_UART	 "/dev/ttyS1"
#define ICDEV_BAUD	 9600

#define ICDEV_TIMEO  150

static int icdev = -1;
static fd_set icdev_set;



int init_card_uart(void);



unsigned long CardRead();

void close_card_uart(void);




static const unsigned char cmd_find[3] = {0x02, 0x02, 0x26};
static const unsigned char cmd_state[3] = {0x02, 0x0B, 0x0F};
static const unsigned char cmd_read[2] = {0x01, 0x03};
static unsigned char cmd_sound[3] = {0x02, 0x10, 0x00};







static int _read_cmd(unsigned char * buffer, unsigned int size)
{
	int ret = 0;
	int read_size = 0;
	int remain = size;

	while ( read_size < size ) {
		ret = read(icdev, buffer + read_size, remain);
		if ( ret <= 0 ) {

			if ( EINTR == errno ) {
				continue;
			}
			break;
		}
		read_size += ret;
		remain -= ret;
	}
	return read_size;
}


static int _read_timeo(int fd, unsigned char * buffer, unsigned int size, unsigned int to_ms)
{
	 int ret = 0;
	 int read_size = -1;
	 int total_read = 0;
	 struct timeval tmout;

	 tmout.tv_sec = to_ms/1000;
	 tmout.tv_usec = (to_ms%1000)*1000;


	 FD_SET(fd, &icdev_set);

	 while ( total_read < size) {
		 //tmout = tmp;

		 ret = select(fd + 1, &icdev_set, NULL, NULL, &tmout);
		 if ( ret < 0 ){
                         if ( EINTR == errno ) {
                                 continue;
                         } else {
                                 debug("\nerror happen while timeo select \n");
                                 FD_ZERO(&icdev_set);
				 break;
                         }
		 }
		 if (FD_ISSET(fd, &icdev_set)) {
			 read_size = _read_cmd(buffer + total_read, size - total_read);
			// debug("read from card = %d ", read_size);
		 } else {	//tmout时间内没有数据
			 break;
		 }
		 total_read += read_size;
	 }
	 FD_CLR(fd, &icdev_set);


	 return total_read;
}

static int _write_cmd(const unsigned char * buffer, unsigned int size)
{
    int ret;

    if ( NULL == buffer )
        return -1;

    do {
		ret = write(icdev, buffer, size);
		if ( ret < 0 ) {
			if ( errno == EINTR ) {
				continue;
			}
			return -1;
		}
	} while (ret < 0);
    return ret;
}



static int _find_card(void)
{
	unsigned char buffer[100];
	int ret = 0;

	//read(icdev, buffer, 100);
	_write_cmd(cmd_find, sizeof(cmd_find));

	ret = _read_timeo(icdev, buffer, 4, ICDEV_TIMEO);
	if ( 4 != ret ) {
		//debug("_read_cmd %d", ret);
		return -1;
	}

	if ( 0x00 != buffer[1] ) {
                debug("find card error %d", buffer[1]);
		return -1;
	}

	ret = _write_cmd(cmd_state, sizeof(cmd_state));
	if (  sizeof(cmd_state) != ret ) {
		//debug("\n_write_cmd\n");
		return -1;
	}

	_read_timeo(icdev, buffer, 100, ICDEV_TIMEO);
        printf("\n ret = %d\n", ret);
	return 0;
}


static int SetNonBlock(int fdListen)
{
    int opts;
    if ((opts = fcntl(fdListen, F_GETFL)) < 0)
    {
        perror("fcntl(F_GETFL) error\n");
        return -1;
    }
    opts |= O_NONBLOCK;
    if (fcntl(fdListen, F_SETFL, opts) < 0)
    {
        perror("fcntl(F_SETFL) error\n");
        return -1;
    }
    return 0;
}

static int _read_card(unsigned char  * pdata, int size)
{
	unsigned char buffer[100];
	int ret = 0;

	if ( NULL == pdata || size < 4)
		return -1;

	if ( _find_card() < 0 ) {
		return -1;
	} else {
#ifdef  DEBUG
                printf("\nFind card ok\n");
#endif
	}

	read(icdev, buffer, 100);
	ret = _write_cmd(cmd_read, sizeof(cmd_read));
	if (  ret != sizeof(cmd_read) ) {
#ifdef  DEBUG
                debug("\n_write_cmd\n");
#endif
		return -1;
	}

	ret = _read_timeo(icdev, buffer, 6, ICDEV_TIMEO);
	if ( 6 != ret ) {
#ifdef  DEBUG
                debug("\nread timeo ret = %d buffer[0] = %d", ret, buffer[0]);
#endif
		return -1;
	}

	if ( 5 != buffer[0] || 0 != buffer[1]) {
#ifdef  DEBUG
                debug("\nread card error\n");
#endif
		return -1;
	}

        debug("\nreads card:%x %x %x %x %x %x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

	pdata[0] = buffer[2];
	pdata[1] = buffer[3];
	pdata[2] = buffer[4];
	pdata[3] = buffer[5];


	ret = _write_cmd(cmd_state, sizeof(cmd_state));
	if (  sizeof(cmd_state) != ret ) {
#ifdef  DEBUG
		debug("\n_write_cmd\n");
#endif
		return -1;
	}
	_read_timeo(icdev, buffer, 100, ICDEV_TIMEO);

	return 0;
}

void close_card_uart(void)
{
	if ( icdev > 0)
		close(icdev);
        icdev = -1;
}

int init_card_uart(void)
{
	int fd = 0;
	
	if ( icdev > 0)
		close(icdev);

	fd = init_serial(ICDEV_UART, ICDEV_BAUD);
        if ( fd <= 0 ) {
		perror("\ninit icdev\n");
		return -1;
	}
	icdev = fd;

	if (SetNonBlock(fd) < 0) {
		close(fd);
		return -1;
	}
	FD_ZERO(&icdev_set);

	return 0;
}

unsigned long CardRead()
{
	unsigned char buffer[4];
	unsigned long card = 0;

	if ( _read_card(buffer, 4) < 0)
        {
            return 0;
        }

        card += buffer[0];
        card += (buffer[1] << 8);
        card += (buffer[2] << 16);
        card += (buffer[3] << 24);

        printf("\n card byte : %x %x %x %x\n", buffer[3], buffer[2], buffer[1], buffer[0]);

	return card;
}

void card_beep(unsigned char ms)
{
        int ret,i;
	unsigned char buffer[100];

	read(icdev, buffer, 100);
#ifdef  DEBUG
        for(i=0;i<100;i++)
        {
            printf("buffer[%d]=%d\n",i,buffer[i]);
        }
#endif
	cmd_sound[2] = ms;
	ret = _write_cmd(cmd_sound, sizeof(cmd_sound));
	if (  ret != sizeof(cmd_sound) ) {
#ifdef  DEBUG
                debug("\n_write_cmd\n");
#endif
		return ;
	}
	_read_timeo(icdev, buffer, 2, ICDEV_TIMEO);

	ret = _write_cmd(cmd_state, sizeof(cmd_state));
	if (  ret != sizeof(cmd_state) ) {
#ifdef  DEBUG
                debug("\n_write_cmd\n");
#endif
		return ;
	}
	_read_timeo(icdev, buffer, 2, ICDEV_TIMEO);
        usleep(20000);
	read(icdev, buffer, 100);
}


#endif


/*
int main()
{
	printf("\n test card \n");
	init_card_uart();
	unsigned int data;


	int size;

	while (1) {
		usleep(10000);

		data = CardRead();

		if ( data != -1) {
			printf("\ndata = %x\n", data);
			cardBeep(20);
			sleep(1);
			sleep(1);
			cardBeep(30);
			sleep(1);
			cardBeep(100);
		}
	}
	return 0;
}*/















































