/*
 * nuc951_rtc.h
 *
 *  Created on: 2013Äê12ÔÂ28ÈÕ
 *      Author: Administrator
 */

#ifndef NUC951_RTC_H_
#define NUC951_RTC_H_


int init_ds3231();
void get_time(struct rtc_time * tm);
int  set_time(unsigned int year0,unsigned int year1,unsigned int month,unsigned int day,unsigned int hour,unsigned int min,unsigned int sec);


#endif /* NUC951_RTC_H_ */
