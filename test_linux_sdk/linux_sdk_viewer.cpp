#include "linux_sdk_viewer.h"
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 전역 변수
LinuxSDKViewer *g_viewer = NULL;
volatile int g_running = 1;

// 유틸리티 함수
int xioctl(int fd, int request, void *arg) {
    int r;
    do {
        r = ioctl(fd, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

int errnoexit(const char *s) {
    printf("%s error %d, %s\n", s, errno, strerror(errno));
    return -1;
}

// LinuxSDKViewer 생성자
LinuxSDKViewer::LinuxSDKViewer() {
    vd = NULL;
    display = NULL;
    window = 0;
    gc = 0;
    ximage = NULL;
    frame_buffer = NULL;
    frame_buffer_size = 0;
    frame_width = 0;
    frame_height = 0;
    h264_fmt = NULL;
    h264_decoder_initialized = 0;
    running = 0;
    
    // FPS 컨트롤러 초기화
    memset(&fps_ctrl, 0, sizeof(FPSController));
    fps_ctrl.target_fps = 30;
    
    // 통계 초기화
    memset(&stats, 0, sizeof(stats));
    clock_gettime(CLOCK_MONOTONIC, &stats.session_start);
    
    // 뮤텍스 초기화
    pthread_mutex_init(&frame_mutex, NULL);
}

// LinuxSDKViewer 소멸자
LinuxSDKViewer::~LinuxSDKViewer() {
    cleanup();
    pthread_mutex_destroy(&frame_mutex);
}

// 초기화
int LinuxSDKViewer::initialize(const CameraConfig *cfg) {
    if (!cfg) return -1;
    
    memcpy(&config, cfg, sizeof(CameraConfig));
    
    printf("=== Linux SDK Viewer 초기화 ===\n");
    printf("장치: %s\n", config.device_name);
    printf("해상도: %dx%d\n", config.width, config.height);
    printf("FPS: %d\n", config.fps);
    printf("포맷: 0x%08X\n", config.format);
    printf("==============================\n");
    
    // 카메라 열기
    if (openCamera(config.device_name) < 0) {
        printf("카메라 열기 실패\n");
        return -1;
    }
    
    // 포맷 설정
    if (setFormat(config.width, config.height, config.fps) < 0) {
        printf("포맷 설정 실패\n");
        return -1;
    }
    
    // H.264 파라미터 설정 (H.264 포맷인 경우)
    if (config.format == V4L2_PIX_FMT_H264) {
        if (setH264Parameters(config.bitrate, config.quality, 30) < 0) {
            printf("H.264 파라미터 설정 실패\n");
            return -1;
        }
    }
    
    // X11 디스플레이 초기화
    if (initializeX11Display() < 0) {
        printf("X11 디스플레이 초기화 실패\n");
        return -1;
    }
    
    // 윈도우 생성
    if (createWindow(config.width, config.height) < 0) {
        printf("윈도우 생성 실패\n");
        return -1;
    }
    
    // FPS 컨트롤러 설정
    setTargetFPS(config.fps);
    
    printf("초기화 완료!\n");
    return 0;
}

// 카메라 열기
int LinuxSDKViewer::openCamera(const char *device) {
    struct stat st;
    
    if (-1 == stat(device, &st)) {
        printf("Cannot identify '%s': %d, %s\n", device, errno, strerror(errno));
        return -1;
    }
    
    if (!S_ISCHR(st.st_mode)) {
        printf("%s is no device\n", device);
        return -1;
    }
    
    vd = (struct vdIn *)calloc(1, sizeof(struct vdIn));
    if (!vd) {
        printf("메모리 할당 실패\n");
        return -1;
    }
    
    vd->fd = open(device, O_RDWR);
    if (-1 == vd->fd) {
        printf("Cannot open '%s': %d, %s\n", device, errno, strerror(errno));
        return -1;
    }
    
    printf("카메라 %s 열기 성공\n", device);
    return 0;
}

// 포맷 설정
int LinuxSDKViewer::setFormat(int width, int height, int fps) {
    if (!vd) return -1;
    
    printf("포맷 설정: %dx%d @ %dfps\n", width, height, fps);
    
    // V4L2 포맷 설정
    struct v4l2_format fmt;
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = config.format;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;
    
    if (-1 == xioctl(vd->fd, VIDIOC_S_FMT, &fmt)) {
        printf("VIDIOC_S_FMT 실패\n");
        return -1;
    }
    
    // 실제 설정된 값 확인
    printf("실제 설정: %dx%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
    
    // FPS 설정
    struct v4l2_streamparm streamparm;
    CLEAR(streamparm);
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparm.parm.capture.timeperframe.numerator = 1;
    streamparm.parm.capture.timeperframe.denominator = fps;
    
    if (-1 == xioctl(vd->fd, VIDIOC_S_PARM, &streamparm)) {
        printf("FPS 설정 실패, 기본값 사용\n");
    } else {
        printf("FPS 설정: %d/%d\n", 
               streamparm.parm.capture.timeperframe.numerator,
               streamparm.parm.capture.timeperframe.denominator);
    }
    
    frame_width = fmt.fmt.pix.width;
    frame_height = fmt.fmt.pix.height;
    
    return 0;
}

// H.264 파라미터 설정
int LinuxSDKViewer::setH264Parameters(int bitrate, int quality, int keyframe_interval) {
    if (!vd) return -1;
    
    printf("H.264 파라미터 설정: bitrate=%d, quality=%d, keyframe=%d\n", 
           bitrate, quality, keyframe_interval);
    
    // Linux SDK의 H.264 XU 컨트롤 사용
    // 실제 구현에서는 h264_xu_ctrls.c의 함수들을 사용
    
    return 0;
}

// FPS 설정
int LinuxSDKViewer::setTargetFPS(int fps) {
    if (fps < MIN_FPS || fps > MAX_FPS) return -1;
    
    fps_ctrl.target_fps = fps;
    fps_ctrl.frame_interval.tv_sec = 0;
    fps_ctrl.frame_interval.tv_nsec = 1000000000 / fps;  // 나노초 단위
    
    printf("목표 FPS 설정: %d (간격: %ld ns)\n", fps, fps_ctrl.frame_interval.tv_nsec);
    return 0;
}

// 현재 FPS 조회
int LinuxSDKViewer::getCurrentFPS() {
    return (int)stats.current_fps;
}

// FPS 제어 업데이트
void LinuxSDKViewer::updateFPSControl() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    // 프레임 카운트 증가
    fps_ctrl.frame_count++;
    stats.total_frames++;
    
    // FPS 계산
    struct timespec elapsed;
    elapsed.tv_sec = now.tv_sec - fps_ctrl.start_time.tv_sec;
    elapsed.tv_nsec = now.tv_sec - fps_ctrl.start_time.tv_nsec;
    
    if (elapsed.tv_nsec < 0) {
        elapsed.tv_sec--;
        elapsed.tv_nsec += 1000000000;
    }
    
    double elapsed_seconds = elapsed.tv_sec + elapsed.tv_nsec / 1000000000.0;
    if (elapsed_seconds > 0) {
        stats.current_fps = fps_ctrl.frame_count / elapsed_seconds;
        stats.avg_fps = stats.total_frames / 
                       ((now.tv_sec - stats.session_start.tv_sec) + 
                        (now.tv_sec - stats.session_start.tv_sec) / 1000000000.0);
    }
}

// 다음 프레임까지 대기
void LinuxSDKViewer::waitForNextFrame() {
    struct timespec now, target;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    // 목표 시간 계산
    target.tv_sec = fps_ctrl.last_frame_time.tv_sec + fps_ctrl.frame_interval.tv_sec;
    target.tv_nsec = fps_ctrl.last_frame_time.tv_nsec + fps_ctrl.frame_interval.tv_nsec;
    
    if (target.tv_nsec >= 1000000000) {
        target.tv_sec++;
        target.tv_nsec -= 1000000000;
    }
    
    // 대기 시간 계산
    struct timespec sleep_time;
    sleep_time.tv_sec = target.tv_sec - now.tv_sec;
    sleep_time.tv_nsec = target.tv_nsec - now.tv_nsec;
    
    if (sleep_time.tv_nsec < 0) {
        sleep_time.tv_sec--;
        sleep_time.tv_nsec += 1000000000;
    }
    
    // 대기
    if (sleep_time.tv_sec > 0 || sleep_time.tv_nsec > 0) {
        nanosleep(&sleep_time, NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &fps_ctrl.last_frame_time);
}

// 스트리밍 시작
int LinuxSDKViewer::startStreaming() {
    if (running) return 0;
    
    printf("스트리밍 시작...\n");
    
    // 버퍼 요청
    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count = MAX_BUFFERS;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    
    if (-1 == xioctl(vd->fd, VIDIOC_REQBUFS, &req)) {
        printf("VIDIOC_REQBUFS 실패\n");
        return -1;
    }
    
    // 버퍼 매핑
    for (int i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (-1 == xioctl(vd->fd, VIDIOC_QUERYBUF, &buf)) {
            printf("VIDIOC_QUERYBUF 실패\n");
            return -1;
        }
        
        vd->mem[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, 
                          MAP_SHARED, vd->fd, buf.m.offset);
        
        if (MAP_FAILED == vd->mem[i]) {
            printf("mmap 실패\n");
            return -1;
        }
    }
    
    // 스트리밍 시작
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(vd->fd, VIDIOC_STREAMON, &type)) {
        printf("VIDIOC_STREAMON 실패\n");
        return -1;
    }
    
    running = 1;
    clock_gettime(CLOCK_MONOTONIC, &fps_ctrl.start_time);
    clock_gettime(CLOCK_MONOTONIC, &fps_ctrl.last_frame_time);
    
    printf("스트리밍 시작 완료\n");
    return 0;
}

// 스트리밍 정지
int LinuxSDKViewer::stopStreaming() {
    if (!running) return 0;
    
    printf("스트리밍 정지...\n");
    
    running = 0;
    
    // 스트리밍 정지
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(vd->fd, VIDIOC_STREAMOFF, &type)) {
        printf("VIDIOC_STREAMOFF 실패\n");
    }
    
    // 버퍼 언매핑
    for (int i = 0; i < MAX_BUFFERS; i++) {
        if (vd->mem[i]) {
            munmap(vd->mem[i], 0);  // 길이는 드라이버가 관리
            vd->mem[i] = NULL;
        }
    }
    
    printf("스트리밍 정지 완료\n");
    return 0;
}

// 프레임 캡처
int LinuxSDKViewer::captureFrame() {
    if (!running) return -1;
    
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    
    // 버퍼 가져오기
    if (-1 == xioctl(vd->fd, VIDIOC_DQBUF, &buf)) {
        if (errno == EAGAIN) {
            // 버퍼가 비어있음
            return -1;
        }
        printf("VIDIOC_DQBUF 실패\n");
        return -1;
    }
    
    // 프레임 데이터 처리
    pthread_mutex_lock(&frame_mutex);
    
    if (config.format == V4L2_PIX_FMT_H264) {
        // H.264 디코딩
        decodeH264Frame((unsigned char*)vd->mem[buf.index], buf.bytesused);
    } else {
        // 직접 복사 (MJPEG, YUV 등)
        if (frame_buffer_size < buf.bytesused) {
            frame_buffer = (unsigned char*)realloc(frame_buffer, buf.bytesused);
            frame_buffer_size = buf.bytesused;
        }
        memcpy(frame_buffer, vd->mem[buf.index], buf.bytesused);
    }
    
    pthread_mutex_unlock(&frame_mutex);
    
    // 버퍼 반환
    if (-1 == xioctl(vd->fd, VIDIOC_QBUF, &buf)) {
        printf("VIDIOC_QBUF 실패\n");
        return -1;
    }
    
    return 0;
}

// H.264 프레임 디코딩
int LinuxSDKViewer::decodeH264Frame(unsigned char *data, int size) {
    // 실제 구현에서는 FFmpeg 라이브러리나 하드웨어 디코더 사용
    // 여기서는 간단한 예시만 제공
    
    printf("H.264 프레임 디코딩: %d bytes\n", size);
    
    // NAL 유닛 파싱 (Linux SDK의 nalu.c 사용)
    // 실제 구현에서는 h264_xu_ctrls.c의 함수들과 연동
    
    return 0;
}

// X11 디스플레이 초기화
int LinuxSDKViewer::initializeX11Display() {
    display = XOpenDisplay(NULL);
    if (!display) {
        printf("X11 디스플레이 열기 실패\n");
        return -1;
    }
    
    printf("X11 디스플레이 초기화 완료\n");
    return 0;
}

// 윈도우 생성
int LinuxSDKViewer::createWindow(int width, int height) {
    if (!display) return -1;
    
    int screen = DefaultScreen(display);
    Window root = DefaultRootWindow(display);
    
    // 윈도우 생성
    window = XCreateSimpleWindow(display, root, 0, 0, width, height, 
                                1, BlackPixel(display, screen), 
                                WhitePixel(display, screen));
    
    // 윈도우 제목 설정
    XStoreName(display, window, "Linux SDK Viewer");
    
    // 이벤트 마스크 설정
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    
    // 윈도우 표시
    XMapWindow(display, window);
    XFlush(display);
    
    // GC 생성
    gc = XCreateGC(display, window, 0, NULL);
    
    printf("윈도우 생성 완료: %dx%d\n", width, height);
    return 0;
}

// 디스플레이 업데이트
void LinuxSDKViewer::updateDisplay() {
    if (!display || !window) return;
    
    // X11 이벤트 처리
    XEvent event;
    while (XPending(display)) {
        XNextEvent(display, &event);
        switch (event.type) {
            case Expose:
                // 윈도우 다시 그리기
                break;
            case KeyPress:
                // 키 입력 처리
                if (event.xkey.keycode == 9) {  // Escape
                    g_running = 0;
                }
                break;
        }
    }
    
    // 오버레이 그리기
    drawOverlay();
    
    XFlush(display);
}

// 오버레이 그리기
void LinuxSDKViewer::drawOverlay() {
    if (!display || !window || !gc) return;
    
    char info_text[256];
    snprintf(info_text, sizeof(info_text), 
             "FPS: %.1f (Target: %d) | Frames: %lu | Size: %dx%d",
             stats.current_fps, fps_ctrl.target_fps, 
             stats.total_frames, frame_width, frame_height);
    
    // 텍스트 그리기
    XDrawString(display, window, gc, 10, 20, info_text, strlen(info_text));
}

// 통계 업데이트
void LinuxSDKViewer::updateStatistics() {
    updateFPSControl();
}

// 통계 출력
void LinuxSDKViewer::printStatistics() {
    printf("\n=== 세션 통계 ===\n");
    printf("총 프레임: %lu\n", stats.total_frames);
    printf("드롭된 프레임: %lu\n", stats.dropped_frames);
    printf("평균 FPS: %.2f\n", stats.avg_fps);
    printf("현재 FPS: %.2f\n", stats.current_fps);
    printf("목표 FPS: %d\n", fps_ctrl.target_fps);
    printf("================\n");
}

// 통계 리셋
void LinuxSDKViewer::resetStatistics() {
    memset(&stats, 0, sizeof(stats));
    fps_ctrl.frame_count = 0;
    clock_gettime(CLOCK_MONOTONIC, &stats.session_start);
    clock_gettime(CLOCK_MONOTONIC, &fps_ctrl.start_time);
    printf("통계 리셋 완료\n");
}

// 정리
void LinuxSDKViewer::cleanup() {
    stopStreaming();
    
    if (vd) {
        if (vd->fd >= 0) {
            close(vd->fd);
        }
        free(vd);
        vd = NULL;
    }
    
    if (frame_buffer) {
        free(frame_buffer);
        frame_buffer = NULL;
    }
    
    if (display) {
        if (window) {
            XDestroyWindow(display, window);
        }
        if (gc) {
            XFreeGC(display, gc);
        }
        XCloseDisplay(display);
        display = NULL;
    }
    
    printf("정리 완료\n");
}

// 시그널 핸들러
void LinuxSDKViewer::signalHandler(int sig) {
    printf("\n시그널 %d 수신, 종료 중...\n", sig);
    g_running = 0;
}

// 사용법 출력
void printUsage(const char *program_name) {
    printf("사용법: %s [옵션]\n", program_name);
    printf("옵션:\n");
    printf("  -d <device>     카메라 장치 (기본: /dev/video0)\n");
    printf("  -w <width>      해상도 너비 (기본: 640)\n");
    printf("  -h <height>     해상도 높이 (기본: 480)\n");
    printf("  -f <fps>        FPS (기본: 30)\n");
    printf("  -b <bitrate>    비트레이트 (H.264용, 기본: 1000000)\n");
    printf("  -q <quality>    품질 (H.264용, 기본: 80)\n");
    printf("  -F <format>     포맷 (0x00000021=H.264, 0x47504A4D=MJPEG)\n");
    printf("  -v              상세 출력\n");
    printf("  -?              이 도움말\n");
}

// 명령행 파싱
int parseCommandLine(int argc, char **argv, CameraConfig *config) {
    int opt;
    
    // 기본값 설정
    strcpy(config->device_name, "/dev/video0");
    config->width = 640;
    config->height = 480;
    config->fps = 30;
    config->format = V4L2_PIX_FMT_H264;  // H.264 기본
    config->bitrate = 1000000;
    config->quality = 80;
    
    while ((opt = getopt(argc, argv, "d:w:h:f:b:q:F:v?")) != -1) {
        switch (opt) {
            case 'd':
                strncpy(config->device_name, optarg, sizeof(config->device_name)-1);
                break;
            case 'w':
                config->width = atoi(optarg);
                break;
            case 'h':
                config->height = atoi(optarg);
                break;
            case 'f':
                config->fps = atoi(optarg);
                break;
            case 'b':
                config->bitrate = atoi(optarg);
                break;
            case 'q':
                config->quality = atoi(optarg);
                break;
            case 'F':
                config->format = strtol(optarg, NULL, 16);
                break;
            case 'v':
                // 상세 출력 플래그
                break;
            case '?':
                printUsage(argv[0]);
                return -1;
            default:
                printUsage(argv[0]);
                return -1;
        }
    }
    
    return 0;
}
