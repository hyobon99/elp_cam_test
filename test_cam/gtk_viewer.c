#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <linux/videodev2.h>
#include <gtk/gtk.h>

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

static gboolean update_image(GtkWidget *drawing_area, struct vdIn *cam, uint8_t *rgb_buffer) {
    // 프레임 읽기
    int ret = uvcGrab(cam);
    if (ret < 0) {
        printf("Failed to grab frame\n");
        return G_SOURCE_REMOVE;
    }
    
    // YUYV to RGB 변환
    yuyv_to_rgb(cam->pFramebuffer, rgb_buffer, cam->width, cam->height);
    
    // GTK 위젯 업데이트
    gtk_widget_queue_draw(drawing_area);
    
    return G_SOURCE_CONTINUE;
}

static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    uint8_t *rgb_buffer = (uint8_t *)data;
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    
    // RGB 데이터를 cairo surface로 변환
    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        rgb_buffer, CAIRO_FORMAT_RGB24, width, height, width * 3);
    
    // 화면에 그리기
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);
    
    cairo_surface_destroy(surface);
    
    return FALSE;
}

static void on_window_destroy(GtkWidget *widget, gpointer data) {
    keep_running = 0;
    gtk_main_quit();
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
    
    // GTK 초기화
    gtk_init(&argc, &argv);
    
    // 윈도우 생성
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "USB Webcam Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    // 드로잉 영역 생성
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, width, height);
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    
    // 카메라 초기화
    struct vdIn cam;
    if (init_videoIn(&cam, device, width, height, pixfmt, 1) < 0) {
        printf("Failed to initialize camera\n");
        return -1;
    }
    
    printf("Camera initialized: %dx%d\n", cam.width, cam.height);
    printf("Press Ctrl+C to quit\n");
    
    // RGB 버퍼 할당
    uint8_t* rgb_buffer = malloc(width * height * 3);
    if (!rgb_buffer) {
        printf("Failed to allocate RGB buffer\n");
        return -1;
    }
    
    // 드로잉 콜백 설정
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_callback), rgb_buffer);
    
    // 타이머 설정 (30fps)
    g_timeout_add(33, (GSourceFunc)update_image, drawing_area);
    
    // 윈도우 표시
    gtk_widget_show_all(window);
    
    // GTK 메인 루프
    gtk_main();
    
    // 정리
    free(rgb_buffer);
    close_v4l2(&cam);
    
    return 0;
} 