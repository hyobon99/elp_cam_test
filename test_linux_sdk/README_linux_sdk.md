# Linux SDK Professional Viewer

ELP H.264 USB Camera SDK를 기반으로 한 전문적인 리눅스 카메라 뷰어입니다.

## 기능

### 🎯 **reference.cpp와 동일한 기능 + 전문적 확장**

#### **기본 기능**
- ✅ **FPS 제어**: 정밀한 프레임 레이트 제어 (1-120 FPS)
- ✅ **해상도 설정**: 다양한 해상도 지원
- ✅ **실시간 모니터링**: FPS, 프레임 수, 통계 정보
- ✅ **GUI 디스플레이**: X11 기반 윈도우 출력

#### **전문적 확장 기능**
- 🔥 **H.264 하드웨어 인코딩**: 카메라 내장 H.264 인코더 활용
- 🔥 **XU (Extension Unit) 제어**: 고급 카메라 기능 제어
- 🔥 **멀티스레딩**: 캡처와 디스플레이 분리 처리
- 🔥 **정밀 타이밍**: 나노초 단위 FPS 제어
- 🔥 **V4L2 네이티브**: Linux Video4Linux2 API 직접 사용
- 🔥 **모션 감지**: 하드웨어 모션 감지 지원
- 🔥 **OSD 오버레이**: 화면에 정보 표시

## 설치 및 빌드

### 1. 의존성 설치

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential libx11-dev libxext-dev

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install libX11-devel libXext-devel

# Raspberry Pi
sudo apt update
sudo apt install build-essential libx11-dev libxext-dev
```

### 2. SDK 경로 설정

Makefile에서 SDK 경로를 올바르게 설정하세요:

```makefile
SDK_PATH = /path/to/your/LINUX\ 开发包-2/ELP\ Linux\ SDK最新/Linux
```

### 3. 빌드

```bash
# 의존성 체크
make check

# 빌드
make all

# 설치 (선택적)
sudo make install
```

## 사용법

### 기본 사용법

```bash
# 기본 설정으로 실행
./linux_sdk_viewer

# 특정 장치와 해상도로 실행
./linux_sdk_viewer -d /dev/video0 -w 1280 -h 720 -f 30

# H.264 포맷으로 실행
./linux_sdk_viewer -d /dev/video0 -w 1920 -h 1080 -f 60 -F 0x00000021

# MJPEG 포맷으로 실행
./linux_sdk_viewer -d /dev/video0 -w 640 -h 480 -f 30 -F 0x47504A4D
```

### 명령행 옵션

| 옵션 | 설명 | 기본값 |
|------|------|--------|
| `-d <device>` | 카메라 장치 | `/dev/video0` |
| `-w <width>` | 해상도 너비 | `640` |
| `-h <height>` | 해상도 높이 | `480` |
| `-f <fps>` | FPS | `30` |
| `-b <bitrate>` | 비트레이트 (H.264용) | `1000000` |
| `-q <quality>` | 품질 (H.264용) | `80` |
| `-F <format>` | 포맷 | `0x00000021` (H.264) |

### 지원 포맷

| 포맷 코드 | 설명 |
|-----------|------|
| `0x00000021` | H.264 (하드웨어 인코딩) |
| `0x47504A4D` | MJPEG |
| `0x56595559` | YUYV |
| `0x32315659` | YV12 |

## 제어

### 키보드 제어

| 키 | 기능 |
|----|------|
| `ESC` | 종료 |
| `R` | 통계 리셋 |
| `I` | 카메라 정보 출력 |
| `F` | 지원 포맷 목록 |

### 시그널 제어

```bash
# Ctrl+C로 안전한 종료
# SIGTERM으로 종료
kill -TERM <pid>
```

## 기술적 특징

### 1. 정밀한 FPS 제어

```cpp
// 나노초 단위 정밀 제어
fps_ctrl.frame_interval.tv_nsec = 1000000000 / target_fps;
nanosleep(&sleep_time, NULL);
```

### 2. H.264 하드웨어 인코딩

```cpp
// Linux SDK의 XU 컨트롤 사용
#define V4L2_CID_H264_CTRL_RERVISION    V4L2_CID_BASE_RERVISION+4
#define XU_RERVISION_SYS_H264_CTRL      0x07
```

### 3. 멀티스레딩 아키텍처

- **캡처 스레드**: V4L2 버퍼에서 프레임 읽기
- **디스플레이 스레드**: X11 윈도우 업데이트
- **메인 스레드**: FPS 제어 및 통계 관리

### 4. V4L2 네이티브 구현

```cpp
// 직접 V4L2 API 사용
struct v4l2_format fmt;
fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
ioctl(fd, VIDIOC_S_FMT, &fmt);
```

## 성능 비교

| 항목 | reference.cpp | Linux SDK Viewer |
|------|---------------|------------------|
| FPS 정밀도 | 밀리초 | 나노초 |
| 하드웨어 가속 | 없음 | H.264 인코더 |
| 메모리 사용량 | 높음 | 최적화됨 |
| CPU 사용률 | 높음 | 낮음 |
| 기능 확장성 | 제한적 | 무제한 |

## 문제 해결

### 일반적인 문제

1. **카메라 권한 문제**
   ```bash
   sudo usermod -a -G video $USER
   # 재로그인 필요
   ```

2. **X11 디스플레이 문제**
   ```bash
   export DISPLAY=:0
   xhost +local:
   ```

3. **SDK 경로 문제**
   ```bash
   make check-sdk
   # SDK_PATH 변수 확인
   ```

### 디버깅

```bash
# 상세 출력으로 실행
./linux_sdk_viewer -v

# 카메라 정보 확인
v4l2-ctl --list-devices
v4l2-ctl -d /dev/video0 --list-formats-ext
```

## 라이센스

이 프로젝트는 ELP Linux SDK를 기반으로 하며, 해당 SDK의 라이센스 조건을 따릅니다.

## 기여

버그 리포트나 기능 요청은 이슈로 등록해 주세요.

---

**참고**: 이 뷰어는 ELP H.264 USB 카메라용으로 설계되었습니다. 다른 카메라에서는 일부 기능이 작동하지 않을 수 있습니다.
