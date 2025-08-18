#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <opencv2/opencv_c.h>
#include <opencv2/highgui/highgui_c.h>

static volatile int keep_running = 1;
static void handle_sigint(int sig) { (void)sig; keep_running = 0; }

int main(int argc, char** argv) {
    const char* device = "/dev/video0";
    int width = 640;
    int height = 480;
    
    if (argc >= 2) device = argv[1];
    if (argc >= 3) width = atoi(argv[2]);
    if (argc >= 4) height = atoi(argv[3]);
    
    signal(SIGINT, handle_sigint);
    
    // OpenCV 카메라 열기
    CvCapture* capture = cvCreateCameraCapture(0);
    if (!capture) {
        printf("Error: Could not open camera device %s\n", device);
        return -1;
    }
    
    // 카메라 설정
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, width);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, height);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 30);
    
    printf("Camera opened successfully!\n");
    printf("Press 'q' to quit, 's' to save frame\n");
    
    // 윈도우 생성
    cvNamedWindow("USB Webcam - Real-time Viewer", CV_WINDOW_AUTOSIZE);
    
    int frame_count = 0;
    clock_t start_time = clock();
    
    while (keep_running) {
        // 프레임 읽기
        IplImage* frame = cvQueryFrame(capture);
        if (!frame) {
            printf("Error: Could not read frame\n");
            break;
        }
        
        frame_count++;
        
        // FPS 계산
        clock_t current_time = clock();
        double elapsed_time = (double)(current_time - start_time) / CLOCKS_PER_SEC;
        double fps = frame_count / elapsed_time;
        
        // 프레임 정보 표시
        char info_text[256];
        snprintf(info_text, sizeof(info_text), "Frame: %d, FPS: %.1f, Size: %dx%d", 
                frame_count, fps, frame->width, frame->height);
        
        CvFont font;
        cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.7, 0.7, 0, 2, CV_AA);
        cvPutText(frame, info_text, cvPoint(10, 30), &font, cvScalar(0, 255, 0, 0));
        
        // 화면에 표시
        cvShowImage("USB Webcam - Real-time Viewer", frame);
        
        // 키 입력 처리
        int key = cvWaitKey(1);
        if (key == 'q' || key == 'Q') {
            break;
        } else if (key == 's' || key == 'S') {
            // 프레임 저장
            char filename[256];
            snprintf(filename, sizeof(filename), "opencv_frame_%06d.jpg", frame_count);
            cvSaveImage(filename, frame, NULL);
            printf("Saved: %s\n", filename);
        }
    }
    
    // 정리
    cvReleaseCapture(&capture);
    cvDestroyAllWindows();
    
    double total_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    printf("Total frames captured: %d\n", frame_count);
    printf("Average FPS: %.1f\n", frame_count / total_time);
    
    return 0;
} 