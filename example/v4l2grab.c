#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <setjmp.h>
#include <linux/videodev2.h>

#include "v4l2grab.h"

#ifdef USBVIDEO

#define  TRUE	0
#define  FALSE	-1

#define FILE_VIDEO 	"/dev/video0"

static   int      fd;
static   struct   v4l2_capability   cap;
struct v4l2_fmtdesc fmtdesc;
struct v4l2_format fmt,fmtack;
struct v4l2_streamparm setfps;  
struct v4l2_requestbuffers req;
struct v4l2_buffer buf;
enum v4l2_buf_type type;
unsigned char frame_buffer[IMAGEWIDTH*IMAGEHEIGHT*3];
 
struct tagBuffer *buffers;

int init_v4l2(int ImageWidth,int ImageHeight)
{
	int i;
	int ret = 0;
	struct v4l2_jpegcompression *jpegcomp_arg;
	
	//opendev
	if ((fd = open(FILE_VIDEO, O_RDWR)) == -1) 
	{
		printf("Error opening V4L interface\n");
		return (FALSE);
	}

	//query cap
	if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) 
	{
		printf("Error opening device %s: unable to query device.\n",FILE_VIDEO);
		return (FALSE);
	}
#ifdef DEBUG_V4L2
	else
        {
     	printf("driver:\t\t%s\n",cap.driver);
     	printf("card:\t\t%s\n",cap.card);
     	printf("bus_info:\t%s\n",cap.bus_info);
     	printf("version:\t%d\n",cap.version);
        printf("capabilities:\t%x\n",cap.capabilities);

     	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) 
     	{
			printf("Device %s: supports capture.\n",FILE_VIDEO);
        }

            if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
            {
                    printf("Device %s: supports streaming.\n",FILE_VIDEO);
            }
        }
#endif
	
	//emu all support fmt
	fmtdesc.index=0;
	fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
#ifdef DEBUG_V4L2
	printf("Support format:\n");
#endif
	while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)
	{
#ifdef DEBUG_V4L2
		printf("\t%d.%s\n",fmtdesc.index+1,fmtdesc.description);
#endif
		fmtdesc.index++;
	}
	

    //set fmt
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
        fmt.fmt.pix.height = ImageHeight;
        fmt.fmt.pix.width = ImageWidth;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	
	if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
	{
            printf("Unable to set format\n");
            return FALSE;
	} 	
	if(ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
	{
            printf("Unable to get format\n");
            return FALSE;
	} 
	{
#ifdef DEBUG_V4L2
            printf("fmt.type:\t\t%d\n",fmt.type);
            printf("pix.pixelformat:\t%c%c%c%c\n",fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,(fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
            printf("pix.height:\t\t%d\n",fmt.fmt.pix.height);
            printf("pix.width:\t\t%d\n",fmt.fmt.pix.width);
            printf("pix.field:\t\t%d\n",fmt.fmt.pix.field);
#endif
	}
	//set fps
	setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	setfps.parm.capture.timeperframe.numerator = 10;
	setfps.parm.capture.timeperframe.denominator = 10;
	
	printf("init %s \t[OK]\n",FILE_VIDEO);

	return TRUE;
}

int v4l2_grab(void)
{
	unsigned int n_buffers;
	//request for 4 buffers 
	req.count=4;
	req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory=V4L2_MEMORY_MMAP;
	if(ioctl(fd,VIDIOC_REQBUFS,&req)==-1)
	{
            printf("request for buffers error\n");
            return FALSE;
	}

	//mmap for buffers
	buffers = malloc(req.count*sizeof (*buffers));
	if (!buffers) 
	{
		printf ("Out of memory\n");
		return(FALSE);
	}
	
	for (n_buffers = 0; n_buffers < req.count; n_buffers++) 
	{
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;
		//query buffers
		if (ioctl (fd, VIDIOC_QUERYBUF, &buf) == -1)
		{
			printf("query buffer error\n");
			return(FALSE);
		}

		buffers[n_buffers].length = buf.length;
		//map
		buffers[n_buffers].start = mmap(NULL,buf.length,PROT_READ |PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if (buffers[n_buffers].start == MAP_FAILED)
		{
			printf("buffer map error\n");
			return(FALSE);
		}
	}

	//queue
	for (n_buffers = 0; n_buffers < req.count; n_buffers++)
	{
		buf.index = n_buffers;
		ioctl(fd, VIDIOC_QBUF, &buf);
	} 
	
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl (fd, VIDIOC_STREAMON, &type);

	ioctl(fd, VIDIOC_DQBUF, &buf);

        printf("grab jpeg OK\n");
	return(TRUE);
}

/*
int yuyv_2_rgb888(void)
{
	int           i,j;
    unsigned char y1,y2,u,v;
    int r1,g1,b1,r2,g2,b2;
    char * pointer;
    
	pointer = buffers[0].start;
	
    for(i=0;i<480;i++)
    {
    	for(j=0;j<320;j++)
    	{
    		y1 = *( pointer + (i*320+j)*4);
    		u  = *( pointer + (i*320+j)*4 + 1);
    		y2 = *( pointer + (i*320+j)*4 + 2);
    		v  = *( pointer + (i*320+j)*4 + 3);
    		
    		r1 = y1 + 1.042*(v-128);
    		g1 = y1 - 0.34414*(u-128) - 0.71414*(v-128);
    		b1 = y1 + 1.772*(u-128);
    		
    		r2 = y2 + 1.042*(v-128);
    		g2 = y2 - 0.34414*(u-128) - 0.71414*(v-128);
    		b2 = y2 + 1.772*(u-128);
    		
    		if(r1>255)
    			r1 = 255;
    		else if(r1<0)
    			r1 = 0;
    		
    		if(b1>255)
    			b1 = 255;
    		else if(b1<0)
    			b1 = 0;	
    		
    		if(g1>255)
    			g1 = 255;
    		else if(g1<0)
    			g1 = 0;	
    			
    		if(r2>255)
    			r2 = 255;
    		else if(r2<0)
    			r2 = 0;
    		
    		if(b2>255)
    			b2 = 255;
    		else if(b2<0)
    			b2 = 0;	
    		
    		if(g2>255)
    			g2 = 255;
    		else if(g2<0)
    			g2 = 0;		
    			
    		*(frame_buffer + ((480-1-i)*320+j)*6    ) = (unsigned char)b1;
    		*(frame_buffer + ((480-1-i)*320+j)*6 + 1) = (unsigned char)g1;
    		*(frame_buffer + ((480-1-i)*320+j)*6 + 2) = (unsigned char)r1;
    		*(frame_buffer + ((480-1-i)*320+j)*6 + 3) = (unsigned char)b2;
    		*(frame_buffer + ((480-1-i)*320+j)*6 + 4) = (unsigned char)g2;
    		*(frame_buffer + ((480-1-i)*320+j)*6 + 5) = (unsigned char)r2;
    	}
    }
    printf("change to RGB OK \n");
}
*/

int close_v4l2(void)
{
     if(fd != -1) 
     {
         close(fd);
         return (TRUE);
     }
     return (FALSE);
}

int get_jpegsize (unsigned char *buf, int insize)
{
     int i;
     for ( i= 1024 ; i< insize; i++) {
            if ((buf[i] == 0xFF) && (buf[i+1] == 0xD9)) return i+2;
             }
     return -1;
}

int YUV2JPEG(const char *outfileName)
{

    FILE * fp1;
    int jpeg_size;
    int ret_grab;
   
    fp1 = fopen(outfileName, "wb");
    if(!fp1)
    {
        perror("open JPEG error\n");
        return(FALSE);
    }
    ret_grab = v4l2_grab();
    if(ret_grab == FALSE)
    {
        close_v4l2();
        init_v4l2(IMAGEWIDTH,IMAGEHEIGHT);
    }

    jpeg_size = get_jpegsize(buffers[0].start,1024*1024);

    printf("jpegsize = %d\n",jpeg_size);
    fwrite(buffers[0].start, jpeg_size, 1, fp1);
    printf("save JPEG OK\n");
    fclose(fp1);
    return(jpeg_size);
}
#endif
