
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <X11/Xlib.h>
#include "v4l2uvc.h"

#include "h264_xu_ctrls.h"
struct H264Format *gH264fmt = NULL;

int Dbg_Param = 0x1f;


#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct v4l2_buffer buf0;
void *mem0[32];
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
char rec_filename[] = "RecordH264.h264";	
FILE *rec_fp1 = NULL;
int errnoexit(const char *s)
{
	printf("%s error %d, %s", s, errno, strerror (errno));
	return -1;
}

int xioctl(int fd, int request, void *arg)
{
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

int open_device(int i)
{

	printf("-------------- open_device-------------------\n");

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

	printf("--------------- init_device------------------\n");

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

	printf("---------------init_mmap------------------\n");

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
	
#if 0
	int i;
	int ret;
	for (i = 0; i < NB_BUFFER; i++) 
	{
		memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
		vd->buf.index = i;
		vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vd->buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(vd->fd, VIDIOC_QUERYBUF, &vd->buf);
		if (ret < 0) {
			printf("Unable to query buffer!\n");
		}
		
		vd->mem[i] = mmap(0 /* start anywhere */ ,
				vd->buf.length, PROT_READ, MAP_SHARED, vd->fd,
				vd->buf.m.offset);
		if (vd->mem[i] == MAP_FAILED) {
			printf("Unable to map buffer!\n");
			
		}

		
	}
#endif

	return 0;
}

int start_previewing(void)
{

	printf("--------------start_previewing-------------------\n");

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

int capturing = 1;

void sigint_handler(int sig)
{
    capturing = 0;
    printf("-----@@@@@ sigint_handler  is over !\n");
    
}

void main()   // -s 1280x720   -d  /dev/video0   -a  test.avi  
{
	char *filename = NULL;
	char *avifilename = NULL;

	int width = 1280; 
	int height = 720;
	
	int i;
		
	avifilename = "test.avi";
	
	signal(SIGINT, sigint_handler);
	int format = V4L2_PIX_FMT_H264;
	
	int ret;
	ret = open_device(1);

	if(ret != -1)
	{
		printf("------open_device--success-- !\n ");
		ret = init_device(width,height,format);
	}
	if(ret != -1)
	{
		printf("------init_device---success------- !\n ");
		ret = start_previewing();
	}

	if(ret != -1)
	{
		printf("---start_previewing------success------- !\n ");
	}

       rec_fp1 = fopen(rec_filename, "a+b");
	

		tdate = localtime (&curdate);
		XU_OSD_Set_CarcamCtrl(vd->fd, 0, 0, 0);
		if(XU_OSD_Set_RTC(vd->fd, tdate->tm_year + 1900, tdate->tm_mon + 1,tdate->tm_mday, tdate->tm_hour, tdate->tm_min, tdate->tm_sec) <0)
		  printf("XU_OSD_Set_RTC_fd = %d Failed\n",fd);
		if(XU_OSD_Set_Enable(vd->fd, 1, 1) <0)
		  printf(" XU_OSD_Set_Enable_fd = %d Failed\n",fd);
	
	
	struct v4l2_buffer buf;
	
	
    
	while(capturing)
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
		
		printf("-----@@@@@  while  to requeue buffer  !\n");
		
	   
		
		fwrite(buffers[buf.index].start, buf.bytesused, 1, rec_fp1);
		
		ret = ioctl(vd->fd, VIDIOC_QBUF, &buf);
		
		if (ret < 0) 
		{
			printf("Unable to requeue buffer");
			exit(1);
		}
	}
	
	
	
	close_v4l2(vd);
	
}
