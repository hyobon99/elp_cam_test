# Raspberry Pi 5 웹캠 뷰어

라즈베리파이5에서 USB 웹캠을 사용하여 실시간 영상을 표시하는 OpenCV 기반 뷰어입니다.

## 주요 개선사항

- **헤드리스 모드 지원**: GUI 없이도 실행 가능
- **라즈베리파이 최적화**: 성능에 맞춘 설정
- **자동 프레임 저장**: 주기적으로 이미지 저장
- **다중 카메라 지원**: 여러 카메라 디바이스 자동 감지
- **성능 모니터링**: 실시간 FPS 및 통계 정보

## 요구사항

- Raspberry Pi 5 (또는 Pi 4)
- USB 웹캠
- OpenCV 4.x
- C++11 지원 컴파일러

## 설치

### 1. 시스템 업데이트
```bash
sudo apt update && sudo apt upgrade -y
```

### 2. 필요한 패키지 설치
```bash
make -f Makefile_rpi install-deps
```

### 3. 빌드
```bash
make -f Makefile_rpi
```

## 사용법

### 기본 실행 (GUI 모드)
```bash
./webcam_viewer_rpi
```

### 헤드리스 모드 (GUI 없이)
```bash
./webcam_viewer_rpi --headless
```

### 특정 해상도로 실행
```bash
./webcam_viewer_rpi /dev/video0 640 360
```

### 자동 프레임 저장
```bash
./webcam_viewer_rpi --headless --save --interval 60
```

### 모든 옵션 사용
```bash
./webcam_viewer_rpi /dev/video1 1280 720 --headless --save --interval 30
```

## 명령행 옵션

- `--headless` 또는 `-h`: GUI 없이 실행 (SSH 접속 시 유용)
- `--save` 또는 `-s`: 자동 프레임 저장 활성화
- `--interval N`: N프레임마다 자동 저장 (기본값: 30)
- `[device]`: 카메라 디바이스 경로 (기본값: /dev/video0)
- `[width]`: 해상도 너비 (기본값: 640)
- `[height]`: 해상도 높이 (기본값: 480)

## 키보드 단축키 (GUI 모드)

- `q` 또는 `Q`: 프로그램 종료
- `s` 또는 `S`: 현재 프레임을 JPG 파일로 저장

## 성능 최적화 팁

### 1. 해상도 조정
라즈베리파이5의 성능에 맞는 해상도 사용:
```bash
# 낮은 해상도 (빠른 처리)
./webcam_viewer_rpi --headless 320 240

# 중간 해상도 (균형)
./webcam_viewer_rpi --headless 640 480

# 높은 해상도 (느린 처리)
./webcam_viewer_rpi --headless 1280 720
```

### 2. FPS 조정
코드에서 기본 FPS를 15로 설정했지만, 필요에 따라 조정 가능:
- 10 FPS: 매우 안정적
- 15 FPS: 권장 설정
- 30 FPS: 고성능 필요

### 3. 메모리 관리
자동 저장 간격을 늘려서 디스크 I/O 줄이기:
```bash
./webcam_viewer_rpi --headless --save --interval 120
```

## 문제 해결

### 카메라가 인식되지 않는 경우
1. USB 웹캠 연결 확인
2. 카메라 권한 확인:
   ```bash
   sudo usermod -a -G video $USER
   ```
3. 사용 가능한 카메라 확인:
   ```bash
   ls -l /dev/video*
   v4l2-ctl --list-devices
   ```

### OpenCV 관련 오류
1. OpenCV 설치 확인:
   ```bash
   make -f Makefile_rpi check-opencv
   ```
2. 재설치:
   ```bash
   make -f Makefile_rpi install-deps
   ```

### 성능 문제
1. CPU 사용량 모니터링:
   ```bash
   htop
   ```
2. 온도 확인:
   ```bash
   vcgencmd measure_temp
   ```
3. 해상도나 FPS 낮추기

### GUI 오류 (X11 관련)
헤드리스 모드 사용:
```bash
./webcam_viewer_rpi --headless
```

## 예제 출력

### GUI 모드
```
Raspberry Pi Webcam Viewer
Device: /dev/video0
Resolution: 640x480
Headless mode: No
Auto save: No
Camera opened successfully!
Actual resolution: 640x480
Actual FPS: 15
Press 'q' to quit, 's' to save frame
```

### 헤드리스 모드
```
Raspberry Pi Webcam Viewer
Device: /dev/video0
Resolution: 640x480
Headless mode: Yes
Auto save: Yes (every 30 frames)
Camera opened successfully!
Actual resolution: 640x480
Actual FPS: 15
Frame: 30, FPS: 15, Size: 640x480
Auto saved: rpi_frame_30.jpg
Frame: 60, FPS: 15, Size: 640x480
Auto saved: rpi_frame_60.jpg
```

## 빌드 옵션

```bash
make -f Makefile_rpi          # 기본 빌드
make -f Makefile_rpi debug    # 디버그 빌드
make -f Makefile_rpi release  # 릴리즈 빌드
make -f Makefile_rpi clean    # 정리
make -f Makefile_rpi install  # 시스템에 설치
```

## 라즈베리파이5 특화 기능

- **ARM Cortex-A76 최적화**: 컴파일러 플래그로 성능 향상
- **메모리 효율성**: 헤드리스 모드로 메모리 사용량 최소화
- **온도 관리**: CPU 사용량을 고려한 FPS 제한
- **다중 카메라**: 여러 USB 카메라 자동 감지 및 전환
