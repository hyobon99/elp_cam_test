#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <linux/videodev2.h>
#include <SDL2/SDL.h>

#include "../Linux_UVC_TestAP/v4l2uvc.h"
#include "../Linux_UVC_TestAP/debug.h"

int Dbg_Param = TESTAP_DBG_ERR;

static volatile int keep_running = 1;
static void handle_sigint(int sig) { (void)sig; keep_running = 0; }

// YUYV to RGB 변환 함수
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
    const char* device = "/dev/video0";
    int width = 640;
    int height = 480;
    unsigned int pixfmt = V4L2_PIX_FMT_YUYV;
    
    if (argc >= 2) device = argv[1];
    if (argc >= 3) width = atoi(argv[2]);
    if (argc >= 4) height = atoi(argv[3]);
    
    signal(SIGINT, handle_sigint);
    
    // SDL 초기화
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    
    // 윈도우 생성
    SDL_Window* window = SDL_CreateWindow("USB Webcam Viewer",
                                        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    
    // 렌더러 생성
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    
    // 텍스처 생성
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                           SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture) {
        printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    
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
    
    SDL_Event event;
    int frame_count = 0;
    
    while (keep_running) {
        // SDL 이벤트 처리
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                keep_running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_q) {
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
        yuyv_to_rgb(cam.pFramebuffer, rgb_buffer, cam.width, cam.height);
        
        // 텍스처 업데이트
        SDL_UpdateTexture(texture, NULL, rgb_buffer, cam.width * 3);
        
        // 화면 지우기
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // 텍스처 렌더링
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        
        // 화면 업데이트
        SDL_RenderPresent(renderer);
        
        // 프레임 레이트 제어
        SDL_Delay(33); // ~30fps
    }
    
    printf("Total frames: %d\n", frame_count);
    
    // 정리
    free(rgb_buffer);
    close_v4l2(&cam);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
} 