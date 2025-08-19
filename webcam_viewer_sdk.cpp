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
        cap.open(device_id);
        if (!cap.isOpened()) {
            std::cout << "Error: Could not open camera device " << device_id << std::endl;
            return false;
        }
        return true;
    }

    bool setFormat(int width, int height, int fps) {
        if (!cap.isOpened()) return false;

        // 해상도 설정
        cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
        
        // FPS 설정
        cap.set(cv::CAP_PROP_FPS, fps);

        // 실제 설정된 값 확인
        current_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        current_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        current_fps = cap.get(cv::CAP_PROP_FPS);

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
        return cap.read(frame);
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
        // 일반적인 해상도 목록
        return {
            {320, 240}, {640, 480}, {800, 600}, {1024, 768},
            {1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}
        };
    }

    std::vector<int> getSupportedFPS() {
        // 일반적인 FPS 값들
        return {15, 24, 25, 30, 60};
    }

    std::vector<int> getSupportedKeyFrameRates() {
        // 일반적인 키프레임 레이트 값들
        return {1, 5, 10, 15, 30, 60};
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

    // OpenCV SDK 컨트롤러 초기화
    OpenCVSDKController controller;
    
    // 카메라 열기
    if (!controller.openCamera(0)) {
        return -1;
    }

    // 카메라 포맷 설정
    controller.setFormat(width, height, fps);
    controller.setKeyFrameRate(keyframe_rate);

    std::cout << "=== OpenCV SDK Style Webcam Viewer ===" << std::endl;
    std::cout << "Camera opened successfully!" << std::endl;
    controller.printCameraInfo();
    controller.printSupportedFormats();
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  'q' - Quit" << std::endl;
    std::cout << "  's' - Save frame" << std::endl;
    std::cout << "  '1-8' - Set resolution (320x240, 640x480, 800x600, 1024x768, 1280x720, 1920x1080, 2560x1440, 3840x2160)" << std::endl;
    std::cout << "  'f1-f5' - Set FPS (15, 24, 25, 30, 60)" << std::endl;
    std::cout << "  'k1-k6' - Set keyframe rate (1, 5, 10, 15, 30, 60)" << std::endl;
    std::cout << "  'i' - Show camera info" << std::endl;
    std::cout << "  'f' - Show supported formats" << std::endl;
    std::cout << "  'm' - Reset FPS monitor" << std::endl;
    std::cout << std::endl;

    // 윈도우 생성
    cv::namedWindow("OpenCV SDK Style Viewer", cv::WINDOW_AUTOSIZE);

    FPSMonitor monitor;
    bool show_info = true;

    while (keep_running) {
        // 프레임 읽기
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

            cv::putText(frame, info_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        }

        // 화면에 표시
        cv::imshow("OpenCV SDK Style Viewer", frame);

        // 키 입력 처리
        int key = cv::waitKey(1);
        if (key == 'q' || key == 'Q') {
            break;
        } else if (key == 's' || key == 'S') {
            // 프레임 저장
            std::string filename = "opencv_sdk_frame_" + std::to_string(monitor.getFrameCount()) + ".jpg";
            cv::imwrite(filename, frame);
            std::cout << "Saved: " << filename << std::endl;
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
        } else if (key == '7') {
            controller.setFormat(2560, 1440, controller.getCurrentFPS());
        } else if (key == '8') {
            controller.setFormat(3840, 2160, controller.getCurrentFPS());
        } else if (key == 49) { // F1
            controller.setFormat(controller.getCurrentWidth(), controller.getCurrentHeight(), 15);
        } else if (key == 50) { // F2
            controller.setFormat(controller.getCurrentWidth(), controller.getCurrentHeight(), 24);
        } else if (key == 51) { // F3
            controller.setFormat(controller.getCurrentWidth(), controller.getCurrentHeight(), 25);
        } else if (key == 52) { // F4
            controller.setFormat(controller.getCurrentWidth(), controller.getCurrentHeight(), 30);
        } else if (key == 53) { // F5
            controller.setFormat(controller.getCurrentWidth(), controller.getCurrentHeight(), 60);
        } else if (key == 'k') {
            // 키프레임 레이트 설정 (k1-k6)
            int kf_key = cv::waitKey(0);
            if (kf_key == '1') controller.setKeyFrameRate(1);
            else if (kf_key == '2') controller.setKeyFrameRate(5);
            else if (kf_key == '3') controller.setKeyFrameRate(10);
            else if (kf_key == '4') controller.setKeyFrameRate(15);
            else if (kf_key == '5') controller.setKeyFrameRate(30);
            else if (kf_key == '6') controller.setKeyFrameRate(60);
        } else if (key == 'i' || key == 'I') {
            controller.printCameraInfo();
        } else if (key == 'f' || key == 'F') {
            controller.printSupportedFormats();
        } else if (key == 'm' || key == 'M') {
            monitor.reset();
            std::cout << "FPS monitor reset" << std::endl;
        }
    }

    // 정리
    controller.release();
    cv::destroyAllWindows();

    std::cout << std::endl;
    std::cout << "=== Session Summary ===" << std::endl;
    std::cout << "Total frames captured: " << monitor.getFrameCount() << std::endl;
    std::cout << "Average FPS: " << monitor.getFPS() << std::endl;
    std::cout << "Final resolution: " << controller.getCurrentWidth() << "x" << controller.getCurrentHeight() << std::endl;
    std::cout << "Final FPS: " << controller.getCurrentFPS() << std::endl;
    std::cout << "Final keyframe rate: " << controller.getKeyFrameRate() << std::endl;

    return 0;
}
