/*
 * flash_device.h
 *
 *  Created on: 2013Äê12ÔÂ28ÈÕ
 *      Author: Administrator
 */

#ifndef FLASH_DEVICE_H_
#define FLASH_DEVICE_H_
#define NUC951

int init_at24c02b();
int read_at24c02b(unsigned int addr);
int write_at24c02b(unsigned int addr, int data);
int write_at24c02b_serial(unsigned int addr, unsigned char * buffer, unsigned int size);


#endif /* FLASH_DEVICE_H_ */
