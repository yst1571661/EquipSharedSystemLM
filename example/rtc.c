/*
 * Real Time Clock Driver Test/Example Program
 *
 * Compile with:
 *  gcc -s -Wall -Wstrict-prototypes rtctest.c -o rtctest
 *
 * Copyright (C) 1996, Paul Gortmaker.
 *
 * Released under the GNU General Public License, version 2,
 * included herein by reference.
 *
 */

#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>

#include "nuc951_rtc.h"
#ifdef NUC951

#define RTC_FILE		"/dev/rtc0"

#define RTC_TICK_ON      _IO('p', 0x03)  /* Update int. enable on        */
#define RTC_TICK_OFF     _IO('p', 0x04)  /* ... off                      */
#define RTC_TICK_SET      _IO('p', 0x05)  /* Periodic int. enable on      */
#define RTC_TICK_READ     _IO('p', 0x06)  /* ... off                      */

int rtc_fd = -1;




void getstime(struct tm *tm)
{
    struct rtc_time TM ;
    get_time(&TM);
    tm->tm_year = TM.tm_year;
    tm->tm_mon = TM.tm_mon;
    tm->tm_mday = TM.tm_mday;
    tm->tm_hour = TM.tm_hour;
    tm->tm_min = TM.tm_min;
    tm->tm_sec = TM.tm_sec;
}

int Set_time(unsigned char * time_buf)
{
        int ret;
        char rtc_string[40];
        ret = set_time(time_buf[0],time_buf[1],time_buf[2],time_buf[3],time_buf[4],time_buf[5],time_buf[6]);
        sprintf(rtc_string,"date %02d%02d%02d%02d%02d%02d.%02d",time_buf[2],time_buf[3],time_buf[4],time_buf[5],time_buf[0],time_buf[1],time_buf[6]);
        system(rtc_string);
        return ret;
}




void alarm_func()
{

        printf("\n\n*Alarm interrupt come on,the func running!*\n");

}
void tick_func()
{

        printf("*\n\nTick interrupt come on,the func running!*\n");

}
/*void ctrl_c_signal (int sig)
{
        switch (sig) {
                printf("/n/n **You have input 'ctrl+c',so must be input 'X' exit DEMO!\n\n");
        case SIGTERM:

        case SIGINT:
                // Disable alarm interrupts
                ioctl(rtc_fd, RTC_AIE_OFF, 0);
                ioctl(rtc_fd, RTC_TICK_OFF, 0);
                close(rtc_fd);

                return ;
                break;
        default:

                printf ("What?\n");
        }
}*/

/*
*setup_time():set up the sec,func
*/
// in rtc_tm, valid values are:
// year 1900-
// month 0-11
// day 1-[28, 29, 30, 31]
// hr 0-23
// min, sec 0 -59

int setup_time(int fd, struct rtc_time * tms)
{

        int retval;
        struct rtc_time rtc_tm;

        rtc_tm.tm_year   =   tms->tm_year;
        rtc_tm.tm_mon    =   tms->tm_mon - 1;
        rtc_tm.tm_mday   =   tms->tm_mday;
        rtc_tm.tm_hour   =   tms->tm_hour;
        rtc_tm.tm_min    =   tms->tm_min;
        rtc_tm.tm_sec    =   tms->tm_sec;

        retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);
        if (retval <0) {
                perror("ioctl RTC_SET_TIME  faild!!!\n");
                return -1;
        }

        /*print current time*/
        printf("Adjust current RTC time as: %04d-%02d-%02d %02d:%02d:%02d\n\n",
               rtc_tm.tm_year,
               rtc_tm.tm_mon,
               rtc_tm.tm_mday,
               rtc_tm.tm_hour,
               rtc_tm.tm_min,
               rtc_tm.tm_sec);
        printf("Adjust current RTC time OK!\n");
        return 0;
}

int read_time(int fd, struct rtc_time * tms)
{

        int retval;
        struct rtc_time rtc_tm;

        /*read current time*/

        retval=ioctl(fd,RTC_RD_TIME,&rtc_tm);
        if (retval <0) {
                printf("ioctl RTC_RD_TIME  faild!!!\n");
                return -1;
        }
        /*print current time*/
        printf("Read current RTC time is: %04d-%02d-%02d %02d:%02d:%02d\n\n",
               rtc_tm.tm_year + 1800,
               rtc_tm.tm_mon + 1,
               rtc_tm.tm_mday,
               rtc_tm.tm_hour,
               rtc_tm.tm_min,
               rtc_tm.tm_sec);
        printf("Read current RTC time test OK!\n");

        tms->tm_year = rtc_tm.tm_year;
        tms->tm_mon  = rtc_tm.tm_mon + 1;
        tms->tm_mday = rtc_tm.tm_mday;
        tms->tm_hour = rtc_tm.tm_hour;
        tms->tm_min  = rtc_tm.tm_min;
        tms->tm_sec  = rtc_tm.tm_sec;
        return 0;
}

/*
*setup_alarm():set up the sec,func
*/
void setup_alarm(int fd,int sec,void(*func)())
{

        int retval;
        unsigned long data;
        struct rtc_time rtc_tm;

        /*read current time*/

        retval=ioctl(fd,RTC_RD_TIME,&rtc_tm);
        if (retval <0) {
                printf("ioctl RTC_RD_TIME  faild!!!");
                return ;
        }

        printf("read current time is: %04d-%02d-%02d %02d:%02d:%02d\n\n",
               rtc_tm.tm_year,
               rtc_tm.tm_mon,
               rtc_tm.tm_mday,
               rtc_tm.tm_hour,
               rtc_tm.tm_min,
               rtc_tm.tm_sec);

        rtc_tm.tm_sec = rtc_tm.tm_sec+sec;//alarm to 5 sec in the future
        if (rtc_tm.tm_sec>60) {
                rtc_tm.tm_sec=rtc_tm.tm_sec-60;
                rtc_tm.tm_min=rtc_tm.tm_min+1;
        }
        //rtc_alarmtm.enabled=0;
        //rtc_alarmtm.time=rtc_tm;

        /* Set the alarm to 5 sec in the future */

        retval = ioctl(fd, RTC_ALM_SET, &rtc_tm);
        if (retval <0) {
                printf("ioctl RTC_ALM_SET  faild!!!\n");
                return ;
        }

        /* Read the current alarm settings */

        retval = ioctl(fd, RTC_ALM_READ, &rtc_tm);
        if (retval <0) {
                printf("ioctl  RTC_ALM_READ faild!!!\n");
                return ;
        }

        printf("read current alarm time is: %04d-%02d-%02d %02d:%02d:%02d\n\n",
               rtc_tm.tm_year,
               rtc_tm.tm_mon,
               rtc_tm.tm_mday,
               rtc_tm.tm_hour,
               rtc_tm.tm_min,
               rtc_tm.tm_sec);

        /* Enable alarm interrupts */

        retval = ioctl(fd, RTC_AIE_ON, 0);
        if (retval <0) {
                printf("ioctl  RTC_AIE_ON faild!!!\n");
                return ;
        }

        fprintf(stderr, "Waiting %d seconds for alarm...\n\n",sec);
        fflush(stderr);

        /* This blocks until the alarm ring causes an interrupt */
        retval = read(fd, &data, sizeof(unsigned long));
        if (retval >0) {
                func();
        } else {
                printf("!!!alarm faild!!!\n");
                return ;
        }
        /* Disable alarm interrupts */
        retval = ioctl(fd, RTC_AIE_OFF, 0);
        if (retval == -1) {
                printf("ioctl RTC_AIE_OFF faild!!!\n");
                return ;
        }
        printf("Test second alarm(hour,min,sec) test OK!\n");
}


int rtc_open()
{
	int fd;

	if ( rtc_fd >= 0 ) {
		close(rtc_fd);
	}

	fd = open (RTC_FILE, O_RDONLY);
	if ( fd < 0 ) {
		perror("rtc_open");
		return -1;
	}
	rtc_fd = fd;
	return 0;
}

int init_ds3231()
{
	return rtc_open();
}




void rtc_close()
{
	if ( rtc_fd >= 0 )
		close(rtc_fd);
}

void get_time(struct rtc_time * tm)
{
	char rtc_string[40];

	if ( NULL == tm )
		return;


	read_time(rtc_fd, tm);
	tm->tm_year += 1800;

	printf("\n-----read rtc %02d%02d%02d%02d%d.%02d-----\n",tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_year,tm->tm_sec);
	snprintf(rtc_string, 40, "date %02d%02d%02d%02d%d.%02d",tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_year,tm->tm_sec);
	system(rtc_string);
}




int  set_time(unsigned int year0,unsigned int year1,unsigned int month,unsigned int day,unsigned int hour,unsigned int min,unsigned int sec)
{
	struct rtc_time rtc_tm;

	rtc_tm.tm_year =year1 + 200;
	rtc_tm.tm_mon = month;
	rtc_tm.tm_mday = day;
	rtc_tm.tm_hour = hour;
	rtc_tm.tm_min = min;
	rtc_tm.tm_sec = sec;

	return setup_time(rtc_fd, &rtc_tm);
}


#endif

