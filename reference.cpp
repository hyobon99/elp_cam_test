#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <signal.h>
#include <opencv2/opencv.hpp>

// ===== 설정 영역 (여기서 원하는 값으로 변경하세요) =====
const int TARGET_WIDTH = 1280;    // 원하는 해상도 너비
const int TARGET_HEIGHT = 720;    // 원하는 해상도 높이
const int TARGET_FPS = 30;        // 원하는 FPS (최대 160까지 가능)
// ===================================================

static volatile bool keep_running = true;
static void handle_sigint(int sig) { (void)sig; keep_running = false; }

int main(int argc, char** argv) {
    std::string device = "/dev/video0";
    int width = TARGET_WIDTH;
    int height = TARGET_HEIGHT;
    int target_fps = TARGET_FPS;
    
    // 명령행 인수로 재정의 가능
    if (argc >= 2) device = argv[1];
    if (argc >= 3) width = std::stoi(argv[2]);
    if (argc >= 4) height = std::stoi(argv[3]);
    if (argc >= 5) target_fps = std::stoi(argv[4]);
    
    signal(SIGINT, handle_sigint);
    
    std::cout << "=== 설정된 값 ===" << std::endl;
    std::cout << "해상도: " << width << "x" << height << std::endl;
    std::cout << "목표 FPS: " << target_fps << std::endl;
    std::cout << "=================" << std::endl;
    
    // OpenCV 카메라 열기
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cout << "Error: Could not open camera device " << device << std::endl;
        return -1;
    }
    
    // 카메라 설정
    std::cout << "카메라 설정 중..." << std::endl;
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap.set(cv::CAP_PROP_FPS, target_fps);
    
    // 실제 설정된 값 확인
    int actual_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int actual_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double actual_fps = cap.get(cv::CAP_PROP_FPS);
    
    std::cout << "실제 설정된 값:" << std::endl;
    std::cout << "  해상도: " << actual_width << "x" << actual_height << std::endl;
    std::cout << "  FPS: " << actual_fps << std::endl;
    
    std::cout << "Camera opened successfully!" << std::endl;
    std::cout << "Press 'q' to quit, 's' to save frame" << std::endl;
    
    // 윈도우 생성
    cv::namedWindow("USB Webcam - Real-time Viewer", cv::WINDOW_AUTOSIZE);
    
    int frame_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto last_frame_time = start_time;
    
    // FPS 제어를 위한 프레임 간격 계산 (마이크로초 단위)
    int frame_interval_us = 1000000 / target_fps; // 마이크로초 단위
    
    while (keep_running) {
        // FPS 제어: 프레임 간격만큼 대기
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed_since_last = std::chrono::duration_cast<std::chrono::microseconds>(now - last_frame_time);
        
        if (elapsed_since_last.count() < frame_interval_us) {
            int sleep_us = frame_interval_us - elapsed_since_last.count();
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
        }
        
        // 프레임 읽기
        cv::Mat frame;
        cap >> frame;
        
        if (frame.empty()) {
            std::cout << "Error: Could not read frame" << std::endl;
            break;
        }
        
        frame_count++;
        last_frame_time = std::chrono::high_resolution_clock::now();
        
        // FPS 계산
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        double actual_fps_calculated = frame_count / (elapsed_time.count() / 1000.0);
        
        // 프레임 정보 표시
        std::string info_text = "Frame: " + std::to_string(frame_count) + 
                               ", FPS: " + std::to_string(static_cast<int>(actual_fps_calculated)) + 
                               " (Target: " + std::to_string(target_fps) + ")" +
                               ", Size: " + std::to_string(frame.cols) + "x" + std::to_string(frame.rows);
        
        cv::putText(frame, info_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        
        // 추가 정보 표시
        std::string config_text = "Config: " + std::to_string(width) + "x" + std::to_string(height) + 
                                 " @ " + std::to_string(target_fps) + "fps";
        cv::putText(frame, config_text, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);
        
        // 화면에 표시
        cv::imshow("USB Webcam - Real-time Viewer", frame);
        
        // 키 입력 처리 (FPS 제어를 위해 waitKey 시간 조정)
        int key = cv::waitKey(1);
        if (key == 'q' || key == 'Q') {
            break;
        } else if (key == 's' || key == 'S') {
            // 프레임 저장
            std::string filename = "opencv_frame_" + std::to_string(frame_count) + ".jpg";
            cv::imwrite(filename, frame);
            std::cout << "Saved: " << filename << std::endl;
        }
    }
    
    // 정리
    cap.release();
    cv::destroyAllWindows();
    
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_time);
    double avg_fps = frame_count / (total_time.count() / 1000.0);
    
    std::cout << std::endl;
    std::cout << "=== 실행 결과 ===" << std::endl;
    std::cout << "Total frames captured: " << frame_count << std::endl;
    std::cout << "Average FPS: " << avg_fps << std::endl;
    std::cout << "Target FPS: " << target_fps << std::endl;
    std::cout << "Final resolution: " << width << "x" << height << std::endl;
    std::cout << "=================" << std::endl;
    
    return 0;
} 