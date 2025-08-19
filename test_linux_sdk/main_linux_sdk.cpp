#include "linux_sdk_viewer.h"
#include <signal.h>

// 전역 변수
RaspberryPiViewer *g_viewer = NULL;
volatile int g_running = 1;

// 시그널 핸들러
void signalHandler(int sig) {
    printf("\n시그널 %d 수신, 종료 중...\n", sig);
    g_running = 0;
}

// 메인 함수
int main(int argc, char **argv) {
    CameraConfig config;
    
    printf("=== Raspberry Pi SDK Professional Viewer ===\n");
    printf("ELP H.264 USB Camera SDK 기반\n");
    printf("플랫폼: Raspberry Pi 전용\n");
    printf("============================================\n");
    
    // 명령행 파싱
    if (parseCommandLine(argc, argv, &config) < 0) {
        return -1;
    }
    
    // 시그널 핸들러 설정
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // 라즈베리파이 SDK 뷰어 생성
    g_viewer = new RaspberryPiViewer();
    if (!g_viewer) {
        printf("뷰어 생성 실패\n");
        return -1;
    }
    
    // 초기화
    if (g_viewer->initialize(&config) < 0) {
        printf("초기화 실패\n");
        delete g_viewer;
        return -1;
    }
    
    // 스트리밍 시작
    if (g_viewer->startStreaming() < 0) {
        printf("스트리밍 시작 실패\n");
        delete g_viewer;
        return -1;
    }
    
    printf("\n=== 스트리밍 시작 ===\n");
    printf("제어:\n");
    printf("  ESC - 종료\n");
    printf("  R - 통계 리셋\n");
    printf("  I - 카메라 정보\n");
    printf("  F - 지원 포맷\n");
    printf("====================\n\n");
    
    // 메인 루프
    while (g_running) {
        // 프레임 캡처
        if (g_viewer->captureFrame() == 0) {
            // FPS 제어
            g_viewer->waitForNextFrame();
            
            // 통계 업데이트
            g_viewer->updateStatistics();
            
            // 디스플레이 업데이트
            g_viewer->updateDisplay();
        }
        
        // 키 입력 처리 (X11 이벤트는 updateDisplay에서 처리됨)
        usleep(1000);  // 1ms 대기
    }
    
    printf("\n=== 종료 중 ===\n");
    
    // 통계 출력
    g_viewer->printStatistics();
    
    // 정리
    delete g_viewer;
    g_viewer = NULL;
    
    printf("종료 완료\n");
    return 0;
}
