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
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>

// 플랫폼 감지
#ifdef __APPLE__
    #define PLATFORM_MACOS
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    // macOS에서는 V4L2 대신 AVFoundation 사용 (별도 구현 필요)
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #include <linux/videodev2.h>
    #include <sys/ioctl.h>
    #include <sys/mman.h>
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/Xos.h>
    #include <X11/Xatom.h>
    
    // 라즈베리파이 감지
    #ifdef __arm__ || defined(__aarch64__)
        #define PLATFORM_RASPBERRY_PI
        // 라즈베리파이 특화 설정
    #endif
#else
    #error "지원되지 않는 플랫폼입니다. Linux 또는 macOS만 지원됩니다."
#endif

// Linux SDK 헤더들 (Linux에서만)
#ifdef PLATFORM_LINUX
    #include "v4l2uvc.h"
    #include "h264_xu_ctrls.h"
#endif

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

// Linux SDK 뷰어 클래스
class LinuxSDKViewer {
private:
#ifdef PLATFORM_LINUX
    struct vdIn *vd;
    // X11 디스플레이 관련
    Display *display;
    Window window;
    GC gc;
    XImage *ximage;
#endif

#ifdef PLATFORM_MACOS
    // macOS용 멤버 변수들 (AVFoundation 기반)
    void *av_capture_session;
    void *av_capture_device;
    void *av_capture_output;
#endif

    FPSController fps_ctrl;
    CameraConfig config;
    int running;
    pthread_t capture_thread;
    pthread_t display_thread;
    pthread_mutex_t frame_mutex;
    
    // 프레임 버퍼
    unsigned char *frame_buffer;
    int frame_buffer_size;
    int frame_width;
    int frame_height;
    
    // H.264 관련 (Linux에서만)
#ifdef PLATFORM_LINUX
    struct H264Format *h264_fmt;
    int h264_decoder_initialized;
#endif
    
    // 통계 정보
    struct {
        unsigned long total_frames;
        unsigned long dropped_frames;
        double avg_fps;
        double current_fps;
        struct timespec session_start;
    } stats;

public:
    LinuxSDKViewer();
    ~LinuxSDKViewer();
    
    // 초기화 및 설정
    int initialize(const CameraConfig *cfg);
    int openCamera(const char *device);
    int setFormat(int width, int height, int fps);
    
#ifdef PLATFORM_LINUX
    int setH264Parameters(int bitrate, int quality, int keyframe_interval);
#endif
    
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
    
#ifdef PLATFORM_LINUX
    int decodeH264Frame(unsigned char *data, int size);
#endif
    
    int displayFrame();
    
    // GUI 관련
#ifdef PLATFORM_LINUX
    int initializeX11Display();
    int createWindow(int width, int height);
    void updateDisplay();
    void drawOverlay();
#endif

#ifdef PLATFORM_MACOS
    int initializeMacOSDisplay();
    int createMacOSWindow(int width, int height);
    void updateMacOSDisplay();
    void drawMacOSOverlay();
#endif
    
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
extern LinuxSDKViewer *g_viewer;
extern volatile int g_running;

// 유틸리티 함수들
#ifdef PLATFORM_LINUX
int xioctl(int fd, int request, void *arg);
#endif

int errnoexit(const char *s);
void printUsage(const char *program_name);
int parseCommandLine(int argc, char **argv, CameraConfig *config);

// 플랫폼별 매크로
#ifdef PLATFORM_MACOS
    #define CLEAR(x) memset(&(x), 0, sizeof(x))
    #define V4L2_PIX_FMT_H264 0x34363248  // 'H264'
    #define V4L2_PIX_FMT_MJPEG 0x47504A4D // 'MJPG'
#endif

#ifdef PLATFORM_LINUX
    #define CLEAR(x) memset(&(x), 0, sizeof(x))
#endif

#endif // LINUX_SDK_VIEWER_H
