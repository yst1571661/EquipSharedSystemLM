#include "nuc_config.h"
#ifndef _YUV2BMP_H
#define _YUV2BMP_H

#ifndef USBVIDEO

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	unsigned char b;
	unsigned char g;
	unsigned char r;
}MyRGB;

#ifndef    NULL
	#define    NULL		0
#endif

int YUV2JPEG(unsigned char *pSrcBuffer, long width, long height, const char *fileName);

#endif



#ifdef __cplusplus
}
#endif
#endif
