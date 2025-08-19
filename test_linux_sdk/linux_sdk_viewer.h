#ifndef LINUX_SDK_VIEWER_H
#define LINUX_SDK_VIEWER_H

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
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <math.h>

// 라즈베리파이 전용 설정
#define PLATFORM_RASPBERRY_PI
#define RASPBERRY_PI_ONLY

// Linux SDK 헤더들
#include "v4l2uvc.h"
#include "h264_xu_ctrls.h"

// 설정 상수
#define MAX_DEVICES 10
#define MAX_BUFFERS 16
#define MAX_FPS 120
#define MIN_FPS 1

// FPS 제어 구조체
typedef struct {
    int target_fps;
    int actual_fps;
    struct timespec frame_interval;
    struct timespec last_frame_time;
    int frame_count;
    struct timespec start_time;
} FPSController;

// 해상도 정보
typedef struct {
    int width;
    int height;
    int supported;
} Resolution;

// 카메라 설정
typedef struct {
    char device_name[32];
    int width;
    int height;
    int fps;
    int format;  // V4L2_PIX_FMT_H264, V4L2_PIX_FMT_MJPEG 등
    int quality;
    int bitrate;
} CameraConfig;

// 라즈베리파이 전용 뷰어 클래스
class RaspberryPiViewer {
private:
    struct vdIn *vd;
    FPSController fps_ctrl;
    CameraConfig config;
    int running;
    pthread_t capture_thread;
    pthread_t display_thread;
    pthread_mutex_t frame_mutex;
    
    // X11 디스플레이 관련
    Display *display;
    Window window;
    GC gc;
    XImage *ximage;
    
    // 프레임 버퍼
    unsigned char *frame_buffer;
    int frame_buffer_size;
    int frame_width;
    int frame_height;
    
    // H.264 관련
    struct H264Format *h264_fmt;
    int h264_decoder_initialized;
    
    // 통계 정보
    struct {
        unsigned long total_frames;
        unsigned long dropped_frames;
        double avg_fps;
        double current_fps;
        struct timespec session_start;
    } stats;

public:
    RaspberryPiViewer();
    ~RaspberryPiViewer();
    
    // 초기화 및 설정
    int initialize(const CameraConfig *cfg);
    int openCamera(const char *device);
    int setFormat(int width, int height, int fps);
    int setH264Parameters(int bitrate, int quality, int keyframe_interval);
    
    // FPS 제어
    int setTargetFPS(int fps);
    int getCurrentFPS();
    void updateFPSControl();
    void waitForNextFrame();
    
    // 스트리밍 제어
    int startStreaming();
    int stopStreaming();
    int isStreaming() const { return running; }
    
    // 프레임 처리
    int captureFrame();
    int decodeH264Frame(unsigned char *data, int size);
    int displayFrame();
    
    // GUI 관련
    int initializeX11Display();
    int createWindow(int width, int height);
    void updateDisplay();
    void drawOverlay();
    
    // 통계 및 모니터링
    void updateStatistics();
    void printStatistics();
    void resetStatistics();
    
    // 유틸리티
    int getSupportedResolutions(Resolution *resolutions, int max_count);
    int getSupportedFPS(int *fps_list, int max_count);
    void printCameraInfo();
    void printSupportedFormats();
    
    // 시그널 핸들링
    static void signalHandler(int sig);
    void cleanup();
};

// 전역 변수
extern RaspberryPiViewer *g_viewer;
extern volatile int g_running;

// 유틸리티 함수들
int xioctl(int fd, int request, void *arg);
int errnoexit(const char *s);
void printUsage(const char *program_name);
int parseCommandLine(int argc, char **argv, CameraConfig *config);

// 라즈베리파이 전용 매크로
#define CLEAR(x) memset(&(x), 0, sizeof(x))

#endif // LINUX_SDK_VIEWER_H
