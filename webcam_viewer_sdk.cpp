#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <signal.h>
#include <opencv2/opencv.hpp>

static volatile bool keep_running = true;
static void handle_sigint(int sig) { (void)sig; keep_running = false; }

// OpenCV SDK 스타일 컨트롤러 클래스
class OpenCVSDKController {
private:
    cv::VideoCapture cap;
    int current_fps;
    int current_width;
    int current_height;
    int keyframe_rate;

public:
    OpenCVSDKController() : current_fps(30), current_width(640), current_height(480), keyframe_rate(30) {}

    bool openCamera(int device_id = 0) {
        // 라즈베리파이에서 실제 카메라 장치 찾기
        std::vector<int> camera_devices = {0, 1, 2, 8, 9}; // 실제 카메라 장치들
        
        for (int dev : camera_devices) {
            std::cout << "카메라 장치 " << dev << " 열기 시도 중..." << std::endl;
            cap.open(dev);
            if (cap.isOpened()) {
                // 실제 카메라인지 확인 (해상도가 0이 아닌 경우)
                double width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
                double height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
                
                if (width > 0 && height > 0) {
                    std::cout << "카메라 장치 " << dev << " 성공적으로 열림!" << std::endl;
                    std::cout << "카메라 정보:" << std::endl;
                    std::cout << "  Width: " << width << std::endl;
                    std::cout << "  Height: " << height << std::endl;
                    std::cout << "  FPS: " << cap.get(cv::CAP_PROP_FPS) << std::endl;
                    std::cout << "  Backend: " << cap.get(cv::CAP_PROP_BACKEND) << std::endl;
                    return true;
                } else {
                    std::cout << "장치 " << dev << "는 실제 카메라가 아닙니다 (해상도: " << width << "x" << height << ")" << std::endl;
                    cap.release();
                }
            }
        }
        
        std::cout << "Error: 사용 가능한 실제 카메라 장치를 찾을 수 없습니다" << std::endl;
        std::cout << "다음 명령어로 카메라 장치를 확인하세요:" << std::endl;
        std::cout << "  v4l2-ctl --list-devices" << std::endl;
        std::cout << "  v4l2-ctl -d /dev/video0 --list-formats-ext" << std::endl;
        return false;
    }

    bool setFormat(int width, int height, int fps) {
        if (!cap.isOpened()) return false;

        // 라즈베리파이에서는 설정을 단계별로 적용
        std::cout << "카메라 포맷 설정 중..." << std::endl;
        
        // 해상도 설정
        cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
        
        // FPS 설정
        cap.set(cv::CAP_PROP_FPS, fps);

        // 실제 설정된 값 확인
        current_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        current_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        current_fps = cap.get(cv::CAP_PROP_FPS);

        // 라즈베리파이에서는 FPS가 -1로 나올 수 있으므로 기본값 사용
        if (current_fps <= 0) {
            current_fps = fps;
            std::cout << "Warning: FPS를 가져올 수 없어 기본값 " << fps << " 사용" << std::endl;
        }

        std::cout << "Format set to: " << current_width << "x" << current_height 
                  << " @ " << current_fps << " fps" << std::endl;
        return true;
    }

    bool setKeyFrameRate(int rate) {
        if (rate <= 0) return false;
        
        keyframe_rate = rate;
        // OpenCV에서는 키프레임 레이트를 직접 설정할 수 없지만,
        // 일부 백엔드에서는 지원할 수 있음
        cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('H', '2', '6', '4'));
        
        std::cout << "Keyframe rate set to: " << rate << std::endl;
        return true;
    }

    bool readFrame(cv::Mat& frame) {
        cap >> frame;
        if (frame.empty()) {
            std::cout << "프레임 읽기 실패 - 카메라 상태 확인 중..." << std::endl;
            std::cout << "  카메라 열림 상태: " << (cap.isOpened() ? "열림" : "닫힘") << std::endl;
            if (cap.isOpened()) {
                std::cout << "  현재 해상도: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x" << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
                double fps = cap.get(cv::CAP_PROP_FPS);
                if (fps <= 0) {
                    std::cout << "  현재 FPS: 알 수 없음 (라즈베리파이 V4L2 제한)" << std::endl;
                } else {
                    std::cout << "  현재 FPS: " << fps << std::endl;
                }
            }
        }
        return !frame.empty();
    }

    void release() {
        cap.release();
    }

    int getCurrentFPS() const {
        return current_fps;
    }

    int getCurrentWidth() const {
        return current_width;
    }

    int getCurrentHeight() const {
        return current_height;
    }

    int getKeyFrameRate() const {
        return keyframe_rate;
    }

    std::vector<std::pair<int, int>> getSupportedResolutions() {
        // 라즈베리파이에 적합한 해상도 목록
        return {
            {320, 240}, {640, 480}, {800, 600}, {1024, 768},
            {1280, 720}, {1920, 1080}
        };
    }

    std::vector<int> getSupportedFPS() {
        // 라즈베리파이에 적합한 FPS 값들
        return {15, 24, 25, 30};
    }

    std::vector<int> getSupportedKeyFrameRates() {
        // 일반적인 키프레임 레이트 값들
        return {1, 5, 10, 15, 30};
    }

    void printCameraInfo() {
        if (!cap.isOpened()) {
            std::cout << "Camera not opened" << std::endl;
            return;
        }

        std::cout << "=== OpenCV SDK Camera Information ===" << std::endl;
        std::cout << "Resolution: " << current_width << "x" << current_height << std::endl;
        std::cout << "FPS: " << current_fps << std::endl;
        std::cout << "Keyframe Rate: " << keyframe_rate << std::endl;
        std::cout << "Brightness: " << cap.get(cv::CAP_PROP_BRIGHTNESS) << std::endl;
        std::cout << "Contrast: " << cap.get(cv::CAP_PROP_CONTRAST) << std::endl;
        std::cout << "Saturation: " << cap.get(cv::CAP_PROP_SATURATION) << std::endl;
        std::cout << "Hue: " << cap.get(cv::CAP_PROP_HUE) << std::endl;
        std::cout << "Gain: " << cap.get(cv::CAP_PROP_GAIN) << std::endl;
        std::cout << "Exposure: " << cap.get(cv::CAP_PROP_EXPOSURE) << std::endl;
        std::cout << "Auto Exposure: " << cap.get(cv::CAP_PROP_AUTO_EXPOSURE) << std::endl;
        std::cout << "Auto Focus: " << cap.get(cv::CAP_PROP_AUTOFOCUS) << std::endl;
    }

    void printSupportedFormats() {
        std::cout << "=== Supported Formats ===" << std::endl;
        
        std::cout << "Resolutions: ";
        auto resolutions = getSupportedResolutions();
        for (size_t i = 0; i < resolutions.size(); ++i) {
            std::cout << resolutions[i].first << "x" << resolutions[i].second;
            if (i < resolutions.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;

        std::cout << "FPS values: ";
        auto fps_values = getSupportedFPS();
        for (size_t i = 0; i < fps_values.size(); ++i) {
            std::cout << fps_values[i];
            if (i < fps_values.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;

        std::cout << "Keyframe rates: ";
        auto keyframe_rates = getSupportedKeyFrameRates();
        for (size_t i = 0; i < keyframe_rates.size(); ++i) {
            std::cout << keyframe_rates[i];
            if (i < keyframe_rates.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
};

// FPS 모니터 클래스
class FPSMonitor {
private:
    int frame_count;
    std::chrono::high_resolution_clock::time_point start_time;
    double current_fps;

public:
    FPSMonitor() : frame_count(0), current_fps(0.0) {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void update() {
        frame_count++;
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
        
        if (elapsed.count() > 0) {
            current_fps = frame_count / (elapsed.count() / 1000.0);
        }
    }

    double getFPS() const {
        return current_fps;
    }

    int getFrameCount() const {
        return frame_count;
    }

    void reset() {
        frame_count = 0;
        start_time = std::chrono::high_resolution_clock::now();
        current_fps = 0.0;
    }
};

int main(int argc, char** argv) {
    std::string device = "/dev/video0";
    int width = 640;
    int height = 480;
    int fps = 30;
    int keyframe_rate = 30;

    if (argc >= 2) device = argv[1];
    if (argc >= 3) width = std::stoi(argv[2]);
    if (argc >= 4) height = std::stoi(argv[3]);
    if (argc >= 5) fps = std::stoi(argv[4]);
    if (argc >= 6) keyframe_rate = std::stoi(argv[5]);

    signal(SIGINT, handle_sigint);

    // 라즈베리파이 환경 설정
    #ifdef RASPBERRY_PI
    // X11 디스플레이 오류 방지
    setenv("QT_QPA_PLATFORM", "offscreen", 1);
    setenv("DISPLAY", ":0", 1);
    #endif

    // OpenCV SDK 컨트롤러 초기화
    OpenCVSDKController controller;
    
    // 카메라 열기
    if (!controller.openCamera(0)) {
        std::cout << "카메라를 열 수 없습니다. 다음을 확인하세요:" << std::endl;
        std::cout << "1. USB 카메라가 연결되어 있는지" << std::endl;
        std::cout << "2. 카메라 권한이 있는지 (sudo usermod -a -G video $USER)" << std::endl;
        std::cout << "3. v4l2-utils가 설치되어 있는지 (sudo apt install v4l-utils)" << std::endl;
        return -1;
    }

    // 카메라 포맷 설정
    controller.setFormat(width, height, fps);
    controller.setKeyFrameRate(keyframe_rate);

    std::cout << "=== OpenCV SDK Style Webcam Viewer (Raspberry Pi) ===" << std::endl;
    std::cout << "Camera opened successfully!" << std::endl;
    controller.printCameraInfo();
    controller.printSupportedFormats();
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  'q' - Quit" << std::endl;
    std::cout << "  's' - Save frame" << std::endl;
    std::cout << "  '1-6' - Set resolution (320x240, 640x480, 800x600, 1024x768, 1280x720, 1920x1080)" << std::endl;
    std::cout << "  'f1-f4' - Set FPS (15, 24, 25, 30)" << std::endl;
    std::cout << "  'k1-k5' - Set keyframe rate (1, 5, 10, 15, 30)" << std::endl;
    std::cout << "  'i' - Show camera info" << std::endl;
    std::cout << "  'f' - Show supported formats" << std::endl;
    std::cout << "  'm' - Reset FPS monitor" << std::endl;
    std::cout << std::endl;

    // 라즈베리파이에서는 GUI 없이 콘솔 모드로 실행
    #ifdef RASPBERRY_PI
    std::cout << "라즈베리파이 모드: GUI 없이 콘솔에서 실행됩니다." << std::endl;
    std::cout << "프레임 정보가 콘솔에 출력됩니다." << std::endl;
    #else
    // 윈도우 생성 (데스크톱 환경에서만)
    cv::namedWindow("OpenCV SDK Style Viewer", cv::WINDOW_AUTOSIZE);
    #endif

    FPSMonitor monitor;
    bool show_info = true;
    int frame_interval = 1000 / fps; // ms 단위

    while (keep_running) {
        // 프레임 읽기 (reference.cpp와 동일한 방식)
        cv::Mat frame;
        if (!controller.readFrame(frame)) {
            std::cout << "Error: Could not read frame" << std::endl;
            break;
        }

        monitor.update();

        // 정보 표시
        if (show_info) {
            std::string info_text = "Frame: " + std::to_string(monitor.getFrameCount()) +
                                   ", FPS: " + std::to_string(static_cast<int>(monitor.getFPS())) +
                                   ", Target: " + std::to_string(controller.getCurrentFPS()) +
                                   ", KF: " + std::to_string(controller.getKeyFrameRate()) +
                                   ", Size: " + std::to_string(frame.cols) + "x" + std::to_string(frame.rows);

            #ifdef RASPBERRY_PI
            // 라즈베리파이에서는 콘솔에 출력
            std::cout << "\r" << info_text << std::flush;
            #else
            // 데스크톱에서는 화면에 표시
            cv::putText(frame, info_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
            #endif
        }

        #ifndef RASPBERRY_PI
        // 화면에 표시 (데스크톱 환경에서만)
        cv::imshow("OpenCV SDK Style Viewer", frame);
        #endif

        // 키 입력 처리
        #ifndef RASPBERRY_PI
        int key = cv::waitKey(frame_interval);
        #else
        // 라즈베리파이에서는 키 입력을 기다리지 않고 계속 실행
        int key = -1;
        std::this_thread::sleep_for(std::chrono::milliseconds(frame_interval));
        #endif

        if (key == 'q' || key == 'Q') {
            break;
        } else if (key == 's' || key == 'S') {
            // 프레임 저장
            std::string filename = "opencv_sdk_frame_" + std::to_string(monitor.getFrameCount()) + ".jpg";
            cv::imwrite(filename, frame);
            std::cout << "\nSaved: " << filename << std::endl;
        } else if (key == '1') {
            controller.setFormat(320, 240, controller.getCurrentFPS());
        } else if (key == '2') {
            controller.setFormat(640, 480, controller.getCurrentFPS());
        } else if (key == '3') {
            controller.setFormat(800, 600, controller.getCurrentFPS());
        } else if (key == '4') {
            controller.setFormat(1024, 768, controller.getCurrentFPS());
        } else if (key == '5') {
            controller.setFormat(1280, 720, controller.getCurrentFPS());
        } else if (key == '6') {
            controller.setFormat(1920, 1080, controller.getCurrentFPS());
        } else if (key == 49) { // F1
            controller.setFormat(controller.getCurrentWidth(), controller.getCurrentHeight(), 15);
        } else if (key == 50) { // F2
            controller.setFormat(controller.getCurrentWidth(), controller.getCurrentHeight(), 24);
        } else if (key == 51) { // F3
            controller.setFormat(controller.getCurrentWidth(), controller.getCurrentHeight(), 25);
        } else if (key == 52) { // F4
            controller.setFormat(controller.getCurrentWidth(), controller.getCurrentHeight(), 30);
        } else if (key == 'k') {
            // 키프레임 레이트 설정 (k1-k5)
            int kf_key = cv::waitKey(0);
            if (kf_key == '1') controller.setKeyFrameRate(1);
            else if (kf_key == '2') controller.setKeyFrameRate(5);
            else if (kf_key == '3') controller.setKeyFrameRate(10);
            else if (kf_key == '4') controller.setKeyFrameRate(15);
            else if (kf_key == '5') controller.setKeyFrameRate(30);
        } else if (key == 'i' || key == 'I') {
            controller.printCameraInfo();
        } else if (key == 'f' || key == 'F') {
            controller.printSupportedFormats();
        } else if (key == 'm' || key == 'M') {
            monitor.reset();
            std::cout << "\nFPS monitor reset" << std::endl;
        }
    }

    // 정리
    controller.release();
    #ifndef RASPBERRY_PI
    cv::destroyAllWindows();
    #endif

    std::cout << std::endl;
    std::cout << "=== Session Summary ===" << std::endl;
    std::cout << "Total frames captured: " << monitor.getFrameCount() << std::endl;
    std::cout << "Average FPS: " << monitor.getFPS() << std::endl;
    std::cout << "Final resolution: " << controller.getCurrentWidth() << "x" << controller.getCurrentHeight() << std::endl;
    std::cout << "Final FPS: " << controller.getCurrentFPS() << std::endl;
    std::cout << "Final keyframe rate: " << controller.getKeyFrameRate() << std::endl;

    return 0;
}
