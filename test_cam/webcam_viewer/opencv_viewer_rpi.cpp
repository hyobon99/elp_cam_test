#include <iostream>
#include <string>
#include <chrono>
#include <signal.h>
#include <opencv2/opencv.hpp>

static volatile bool keep_running = true;
static void handle_sigint(int sig) { (void)sig; keep_running = false; }

int main(int argc, char** argv) {
    std::string device = "/dev/video0";
    int width = 640;
    int height = 480;
    bool headless_mode = false;
    bool save_frames = false;
    int save_interval = 30; // 30프레임마다 저장
    
    // 명령행 인수 파싱
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--headless" || arg == "-h") {
            headless_mode = true;
        } else if (arg == "--save" || arg == "-s") {
            save_frames = true;
        } else if (arg == "--interval" && i + 1 < argc) {
            save_interval = std::stoi(argv[++i]);
        } else if (i == 1) {
            device = arg;
        } else if (i == 2) {
            width = std::stoi(arg);
        } else if (i == 3) {
            height = std::stoi(arg);
        }
    }
    
    signal(SIGINT, handle_sigint);
    
    std::cout << "Raspberry Pi Webcam Viewer" << std::endl;
    std::cout << "Device: " << device << std::endl;
    std::cout << "Resolution: " << width << "x" << height << std::endl;
    std::cout << "Headless mode: " << (headless_mode ? "Yes" : "No") << std::endl;
    std::cout << "Auto save: " << (save_frames ? "Yes (every " + std::to_string(save_interval) + " frames)" : "No") << std::endl;
    
    // OpenCV 카메라 열기
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cout << "Error: Could not open camera device " << device << std::endl;
        std::cout << "Trying alternative device..." << std::endl;
        
        // 대안 디바이스 시도
        cap.open("/dev/video1");
        if (!cap.isOpened()) {
            cap.open("/dev/video2");
        }
        
        if (!cap.isOpened()) {
            std::cout << "Error: Could not open any camera device" << std::endl;
            return -1;
        }
    }
    
    // 카메라 설정
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap.set(cv::CAP_PROP_FPS, 15); // 라즈베리파이에서는 낮은 FPS 권장
    
    // 실제 설정된 값 확인
    double actual_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double actual_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double actual_fps = cap.get(cv::CAP_PROP_FPS);
    
    std::cout << "Camera opened successfully!" << std::endl;
    std::cout << "Actual resolution: " << actual_width << "x" << actual_height << std::endl;
    std::cout << "Actual FPS: " << actual_fps << std::endl;
    
    if (!headless_mode) {
        std::cout << "Press 'q' to quit, 's' to save frame" << std::endl;
        cv::namedWindow("Raspberry Pi Webcam Viewer", cv::WINDOW_AUTOSIZE);
    }
    
    int frame_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    while (keep_running) {
        // 프레임 읽기
        cv::Mat frame;
        cap >> frame;
        
        if (frame.empty()) {
            std::cout << "Error: Could not read frame" << std::endl;
            break;
        }
        
        frame_count++;
        
        // FPS 계산
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        double fps = frame_count / (elapsed_time.count() / 1000.0);
        
        // 프레임 정보 표시
        std::string info_text = "Frame: " + std::to_string(frame_count) + 
                               ", FPS: " + std::to_string(static_cast<int>(fps)) + 
                               ", Size: " + std::to_string(frame.cols) + "x" + std::to_string(frame.rows);
        
        if (!headless_mode) {
            cv::putText(frame, info_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
            cv::imshow("Raspberry Pi Webcam Viewer", frame);
        } else {
            // 헤드리스 모드에서는 콘솔에 정보 출력
            if (frame_count % 30 == 0) { // 30프레임마다 출력
                std::cout << info_text << std::endl;
            }
        }
        
        // 자동 저장
        if (save_frames && frame_count % save_interval == 0) {
            std::string filename = "rpi_frame_" + std::to_string(frame_count) + ".jpg";
            cv::imwrite(filename, frame);
            std::cout << "Auto saved: " << filename << std::endl;
        }
        
        // 키 입력 처리 (GUI 모드에서만)
        if (!headless_mode) {
            int key = cv::waitKey(1);
            if (key == 'q' || key == 'Q') {
                break;
            } else if (key == 's' || key == 'S') {
                // 프레임 저장
                std::string filename = "rpi_frame_" + std::to_string(frame_count) + ".jpg";
                cv::imwrite(filename, frame);
                std::cout << "Saved: " << filename << std::endl;
            }
        }
        
        // 헤드리스 모드에서 Ctrl+C 감지
        if (headless_mode) {
            // 짧은 지연으로 CPU 사용량 줄이기
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    // 정리
    cap.release();
    if (!headless_mode) {
        cv::destroyAllWindows();
    }
    
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_time);
    double avg_fps = frame_count / (total_time.count() / 1000.0);
    
    std::cout << "\n=== Session Summary ===" << std::endl;
    std::cout << "Total frames captured: " << frame_count << std::endl;
    std::cout << "Average FPS: " << avg_fps << std::endl;
    std::cout << "Total time: " << (total_time.count() / 1000.0) << " seconds" << std::endl;
    
    return 0;
}
