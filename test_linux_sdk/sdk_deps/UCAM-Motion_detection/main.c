
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include "v4l2uvc.h"
#include "rervision_xu_ctrls.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))


struct buffer {
	void *         start;
	size_t         length;
};

static char            dev_name[16];
static int              fd              = -1;
struct buffer *         buffers         = NULL;
static unsigned int     n_buffers       = 0;

struct vdIn *vd;

struct tm *tdate;
time_t curdate;

int errnoexit(const char *s)
{
	printf("%s error %d, %s", s, errno, strerror (errno));
	return -1;
}

/***************    设置移动侦测    *****************/
static unsigned char md_mask[24] = {0};
static unsigned char md_result[24] = {0};
char *optarg = "ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff", *endptr;
/***************    设置移动侦测    *****************/

static unsigned char dht_data[DHT_SIZE] = {
    0xff, 0xc4, 0x00, 0x1f, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
    0x0b, 0xff, 0xc4, 0x00, 0xb5, 0x10, 0x00, 0x02,
    0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00,
    0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11,
    0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71,
    0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42,
    0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09,
    0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43,
    0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92,
    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,
    0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4,
    0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9, 0xfa, 0xff, 0xc4, 0x00, 0x1f,
    0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0xff, 0xc4, 0x00,
    0xb5, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04,
    0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01,
    0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06,
    0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14,
    0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33,
    0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25,
    0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46,
    0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56,
    0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
    0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94,
    0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
    0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa
};

#define ERROR_LOCAL   -1
#define SUCCESS_LOCAL 0
#define DHT_SIZE      432
#define CACHE_SIZE  3*1024*1024


int xioctl(int fd, int request, void *arg)
{
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

int open_device(int i)
{
	struct stat st;
	sprintf(dev_name,"/dev/video%d",i);
	if (-1 == stat (dev_name, &st))
	{
		printf("Cannot identify '%s': %d, %s", dev_name, errno, strerror (errno));
		return -1;
	}

	if (!S_ISCHR (st.st_mode))
	{
		printf("%s is no device", dev_name);
		return -1;
	}
	vd = (struct vdIn *) calloc(1, sizeof(struct vdIn));
	vd->fd = open (dev_name, O_RDWR);
	
	if (-1 == vd->fd)
	{
		printf("Cannot open '%s': %d, %s", dev_name, errno, strerror (errno));
		return -1;
	}
	return 0;
}

int init_device(int width, int height,int format)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl (vd->fd, VIDIOC_QUERYCAP, &cap))
	{
		if (EINVAL == errno)
		{
			printf("%s is no V4L2 device", dev_name);
			return -1;
		}
		else
		{
			return errnoexit ("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		printf("%s is no video capture device", dev_name);
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		printf("%s does not support streaming i/o", dev_name);
		return -1;
	}

	CLEAR (cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl (vd->fd, VIDIOC_CROPCAP, &cropcap))
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; 

		if (-1 == xioctl (vd->fd, VIDIOC_S_CROP, &crop))
		{
			switch (errno)
			{
				case EINVAL:
					break;
				default:
					break;
			}
		}
	}
	else
	{

	}

	CLEAR (fmt);

	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width;
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = format;
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;

	if (-1 == xioctl (vd->fd, VIDIOC_S_FMT, &fmt))
		return errnoexit ("VIDIOC_S_FMT");

	min = fmt.fmt.pix.width * 2;

	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	return init_mmap ();

}

int init_mmap(void)
{
	struct v4l2_requestbuffers req;

	CLEAR (req);
	req.count               = 4;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (vd->fd, VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			printf("%s does not support memory mapping", dev_name);
			return -1;
		}
		else
		{
			return errnoexit ("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2)
	{
		printf("Insufficient buffer memory on %s", dev_name);
		return -1;
 	}

	buffers = calloc (req.count, sizeof (*buffers));

	if (!buffers)
	{
		printf("Out of memory");
		return -1;
	}
	
	for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
	{
		struct v4l2_buffer buf;
		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl (vd->fd, VIDIOC_QUERYBUF, &buf))
			return errnoexit ("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
		mmap (NULL ,
			buf.length,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			vd->fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
			return errnoexit ("mmap");
	}

	return 0;
}

int start_previewing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i)
	{
		struct v4l2_buffer buf;
		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;

		if (-1 == xioctl (vd->fd, VIDIOC_QBUF, &buf))
			return errnoexit ("VIDIOC_QBUF");
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl (vd->fd, VIDIOC_STREAMON, &type))
		return errnoexit ("VIDIOC_STREAMON");

	return 0;
}

static void get_picture_name (char *Picture, int fmt,int flag)
{
	  char temp[150];
	  char *myext[] = { "bmp", "jpg" };
	  memset (temp, '\0', sizeof (temp));
	  time (&curdate);
	  tdate = localtime (&curdate);

	  snprintf (temp,100,"%04d%02d%02d%02d%02d%02d.%s",tdate->tm_year + 1900,tdate->tm_mon + 1, tdate->tm_mday,tdate->tm_hour,tdate->tm_min,tdate->tm_sec,myext[fmt]);
	  printf("-----@@@@@   图片名字是 %s  !/n ",temp);
	  memcpy(Picture, temp, strlen (temp));
}

static int is_huffman(unsigned char *buf)
{
	unsigned char *ptbuf;
	int i = 0;
	ptbuf = buf;
	while (((ptbuf[0] << 8) | ptbuf[1]) != 0xffda){
		if(i++ > 2048)
			return 0;
		if(((ptbuf[0] << 8) | ptbuf[1]) == 0xffc4)
			return 1;
		ptbuf++;
	}
	return 0;
}


static int get_picture(unsigned char *buf,int size,int flag)
{
	FILE *file;
	unsigned char *ptdeb,*ptcur = buf;
	int sizein;
	char *name = NULL;
	name = calloc(100,1);

	get_picture_name(name, 1,flag);

	file = fopen(name, "wb");
	if(file == NULL)
		printf("--  fopen failed -----!\n");
	if (file != NULL)
	{
		if(!is_huffman(buf))
		{
			ptdeb = ptcur = buf;
			while (((ptcur[0] << 8) | ptcur[1]) != 0xffc0)
					ptcur++;
			sizein = ptcur-ptdeb;

			fwrite(buf,sizein, 1, file);
			fwrite(dht_data,DHT_SIZE, 1, file);
			fwrite(ptcur,size-sizein,1,file);

		}
		else
		{
			fwrite(ptcur,size,1,file);
		}
		fclose(file);

	}
	if(name)
		free(name);
	return 0;
}



static int close_device(int fd)
{
	if (-1 == close (fd))
	{
		fd = -1;
		return errnoexit ("close");
	}
	fd = -1;
	return SUCCESS_LOCAL;
}


void main()
{	
	int width = 640; 
	int height = 480;
	int format  = V4L2_PIX_FMT_MJPEG; 	
	struct v4l2_buffer buf;
	int framesizeIn;
    unsigned char *framebuffer;
	
	int md_capture = 0;
	static int flag_photo = 0;
	
	int ret;	
	int i = 0;

	ret = open_device(0);

	if(ret != -1){
		printf("------open_device--success-- !\n ");
		ret = init_device(width,height,format);
	}
	if(ret != -1){
		printf("------init_device---success------- !\n ");
		ret = start_previewing();
	}

	if(ret != -1){
		printf("---start_previewing------success------- !\n ");
	}
	
	framebuffer = (unsigned char *)calloc(1,(size_t)(width*height<<1));
	framesizeIn = (width * height << 1);
    
    /****************  移动 侦测 相关  ********************/	
	md_mask[0] = strtol(optarg, &endptr, 16);
	
	for(i=1; i<24; i++){
		md_mask[i] = strtol(endptr+1, &endptr, 16);
		printf("----@@@@  md_mask[%d] = %d\n", i, md_mask[i]);
	}
	
	if(XU_MD_Set_Mask(vd->fd, md_mask) <0)
	{
		printf("-----@@@@@ XU_MD_Set_Mask Failed ！\n");
		exit(1);
	}
	/****************  移动 侦测 相关  ********************/		
    
	while(!md_capture)
	{				
		CLEAR (buf);
		
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		ret = ioctl(vd->fd, VIDIOC_DQBUF, &buf);
		if (ret < 0) 
		{
			printf("Unable to dequeue buffer!\n");
			exit(1);
		}
		
		/****************  移动 侦测 拍照  ********************/
	    if( (XU_MD_Get_RESULT(vd->fd, md_mask) > 12) && (flag_photo > 10) )
		{
			printf(" ----@@@@@@   侦测到有物体移动 开始拍照!\n");
			if (buf.bytesused > framesizeIn)
				memcpy(framebuffer, buffers[buf.index].start,(size_t)framesizeIn);
		    else
				memcpy(framebuffer, buffers[buf.index].start,(size_t)buf.bytesused);
			get_picture(framebuffer, buf.bytesused,0);
			usleep(1);
			md_capture = 1;
		}
		/****************  移动 侦测 拍照  ********************/
		
		ret = ioctl(vd->fd, VIDIOC_QBUF, &buf);
		
		if (ret < 0) 
		{
			printf("Unable to requeue buffer");
			exit(1);
		}
		
		flag_photo ++;	
	}
	
	free(framebuffer);
	framebuffer = NULL;
	close_v4l2(vd);
	
}
