#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <linux/videodev2.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "../Linux_UVC_TestAP/v4l2uvc.h"
#include "../Linux_UVC_TestAP/debug.h"

int Dbg_Param = TESTAP_DBG_ERR;

static volatile int keep_running = 1;
static void handle_sigint(int sig) { (void)sig; keep_running = 0; }

// YUYV to RGB 변환 함수 (간단한 버전)
void yuyv_to_rgb(uint8_t *yuyv, uint8_t *rgb, int width, int height) {
    int i, j;
    int y, u, v;
    int r, g, b;
    
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j += 2) {
            // YUYV 포맷: Y0 U Y1 V
            y = yuyv[i * width * 2 + j * 2 + 0];
            u = yuyv[i * width * 2 + j * 2 + 1];
            y = yuyv[i * width * 2 + j * 2 + 2];
            v = yuyv[i * width * 2 + j * 2 + 3];
            
            // YUV to RGB 변환 (첫 번째 픽셀)
            r = y + 1.402 * (v - 128);
            g = y - 0.344 * (u - 128) - 0.714 * (v - 128);
            b = y + 1.772 * (u - 128);
            
            r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
            g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
            b = (b > 255) ? 255 : ((b < 0) ? 0 : b);
            
            rgb[i * width * 3 + j * 3 + 0] = r;
            rgb[i * width * 3 + j * 3 + 1] = g;
            rgb[i * width * 3 + j * 3 + 2] = b;
            
            // 두 번째 픽셀
            y = yuyv[i * width * 2 + j * 2 + 2];
            
            r = y + 1.402 * (v - 128);
            g = y - 0.344 * (u - 128) - 0.714 * (v - 128);
            b = y + 1.772 * (u - 128);
            
            r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
            g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
            b = (b > 255) ? 255 : ((b < 0) ? 0 : b);
            
            rgb[i * width * 3 + (j + 1) * 3 + 0] = r;
            rgb[i * width * 3 + (j + 1) * 3 + 1] = g;
            rgb[i * width * 3 + (j + 1) * 3 + 2] = b;
        }
    }
}

int main(int argc, char** argv) {
    char* device = "/dev/video0";
    int width = 640;
    int height = 480;
    unsigned int pixfmt = V4L2_PIX_FMT_YUYV;
    
    if (argc >= 2) device = argv[1];
    if (argc >= 3) width = atoi(argv[2]);
    if (argc >= 4) height = atoi(argv[3]);
    
    signal(SIGINT, handle_sigint);
    
    // X11 디스플레이 연결
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        printf("Cannot open display\n");
        return -1;
    }
    
    int screen = DefaultScreen(display);
    Window window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                       10, 10, width, height, 1,
                                       BlackPixel(display, screen),
                                       WhitePixel(display, screen));
    
    XSetStandardProperties(display, window, "USB Webcam Viewer", "Webcam", None, NULL, 0, NULL);
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
    
    // 카메라 초기화
    struct vdIn cam;
    if (init_videoIn(&cam, device, width, height, pixfmt, 1) < 0) {
        printf("Failed to initialize camera\n");
        return -1;
    }
    
    printf("Camera initialized: %dx%d\n", cam.width, cam.height);
    printf("Press 'q' to quit\n");
    
    // RGB 버퍼 할당
    uint8_t* rgb_buffer = malloc(width * height * 3);
    if (!rgb_buffer) {
        printf("Failed to allocate RGB buffer\n");
        return -1;
    }
    
    // X11 이미지 생성 (한 번만)
    XImage *image = XCreateImage(display, DefaultVisual(display, screen),
                               DefaultDepth(display, screen), ZPixmap, 0,
                               (char*)rgb_buffer, width, height, 8, 0);
    
    if (!image) {
        printf("Failed to create X11 image\n");
        return -1;
    }
    
    XEvent event;
    int frame_count = 0;
    
    while (keep_running) {
        // X11 이벤트 처리
        while (XPending(display)) {
            XNextEvent(display, &event);
            if (event.type == KeyPress) {
                char key = XLookupKeysym(&event.xkey, 0);
                if (key == 'q' || key == 'Q') {
                    keep_running = 0;
                }
            }
        }
        
        // 프레임 읽기
        int ret = uvcGrab(&cam);
        if (ret < 0) {
            printf("Failed to grab frame\n");
            break;
        }
        
        frame_count++;
        
        // YUYV to RGB 변환
        yuyv_to_rgb(cam.framebuffer, rgb_buffer, cam.width, cam.height);
        
        // 화면에 그리기
        GC gc = DefaultGC(display, screen);
        XPutImage(display, window, gc, image, 0, 0, 0, 0, width, height);
        
        // 프레임 레이트 제어
        usleep(33000); // ~30fps
    }
    
    printf("Total frames: %d\n", frame_count);
    
    // 정리
    XDestroyImage(image);
    free(rgb_buffer);
    close_v4l2(&cam);
    XCloseDisplay(display);
    
    return 0;
} 