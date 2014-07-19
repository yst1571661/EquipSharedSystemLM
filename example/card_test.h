/*
 * card_test.h
 *
 *  Created on: 2013Äê12ÔÂ28ÈÕ
 *      Author: Administrator
 */

#ifndef CARD_TEST_H_
#define CARD_TEST_H_

//#define DEBUG

int init_card_uart(void);
unsigned long CardRead();
void close_card_uart(void);
void card_beep(int ms);

#endif /* CARD_TEST_H_ */
