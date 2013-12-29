/*
 * flash_device.c
 *
 *  Created on: 2013Äê12ÔÂ27ÈÕ
 *      Author: Administrator
 */

#include <stdio.h>
#include <stdlib.h>
#include "flash_device.h"
#ifdef NUC951
#define  FP_FILE	"/var/data/eeprom.dat"


#define  MAX_ADDR	(1024)



int init_at24c02b();
int read_at24c02b(unsigned int addr);
int write_at24c02b(unsigned int addr, int data);
int write_at24c02b_serial(unsigned int addr, unsigned char * buffer, unsigned int size);


static FILE * fp_flash;


int write_at24c02b_serial(unsigned int addr, unsigned char * buffer, unsigned int size)
{
	if ( (unsigned long long)addr + size >= MAX_ADDR || NULL == fp_flash ) {
		return -1;
	}

	if ( fseek(fp_flash, addr, SEEK_SET) < 0 ) {
		printf("\n fseek error \n");
		return -1;
	}

	if ( fwrite(buffer, 1, size, fp_flash) != size) {
		printf("\n fseek error \n");
		return -1;
	}
	fflush(fp_flash);
	return 0;
}


int init_at24c02b()
{
	if ( NULL != fp_flash ) {
		fclose(fp_flash);
	}

	fp_flash = fopen(FP_FILE, "wb+");
	if ( NULL == fp_flash) {
		return -1;
	}

	return 0;
}



int read_at24c02b(unsigned int addr)
{
	unsigned char buf;

	if ( addr >= MAX_ADDR || NULL == fp_flash ) {
		printf("\n para error read_at24c02b\n");
		return -1;
	}

	if ( fseek(fp_flash, addr, SEEK_SET) < 0 ) {
		printf("\n fseek error \n");
		return -1;
	}

	if ( fread(&buf, 1, 1, fp_flash) != 1) {
		printf("\n fseek error \n");
		return -1;
	}
	return buf;
}

int write_at24c02b(unsigned int addr, int data)
{
	if ( addr >= MAX_ADDR || NULL == fp_flash ) {
		printf("\n para error write_at24c02b\n");
		return -1;
	}

	if ( fseek(fp_flash, addr, SEEK_SET) < 0 ) {
		printf("\n fseek error \n");
		return -1;
	}

	if ( fwrite(&data, 1, 1, fp_flash) != 1) {
		printf("\n fseek error \n");
		return -1;
	}
	fflush(fp_flash);
	return 0;
}

#endif

/*int main()
{
	int i;

	init_at24c02b();
	write_at24c02b(20, 5);


	for (i = 0; i < MAX_ADDR; i++) {
		if (write_at24c02b(i, i%256) < 0) {
			printf("\n error happen\n");
			return -1;
		}
		if ( i%256 != read_at24c02b(i) ) {
			printf("\n write error i = %d\n", i);
			return -1;
		} else {
			printf("\n write ok \n");
		}
	}
	printf("\n test end\n");
	return 0;
}*/
