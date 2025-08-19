# Raspberry Pi SDK Professional Viewer

ELP H.264 USB Camera SDK를 기반으로 한 **라즈베리파이 전용** 전문적인 카메라 뷰어입니다.

## 🎯 **라즈베리파이 전용**

이 뷰어는 **라즈베리파이에서만 작동**하도록 설계되었습니다.

### **지원 플랫폼**
- ✅ **Raspberry Pi 3/4/5** (ARM32/ARM64)
- ❌ **macOS** (지원 안됨)
- ❌ **일반 Linux** (지원 안됨)

## 📦 **SDK 의존성**

이 프로젝트는 **로컬 SDK 의존성**을 포함하고 있습니다:

```
sdk_deps/
├── OSD-Linux_H264_AP_0724/          # H.264 인코딩 및 V4L2 제어
│   ├── h264_xu_ctrls.c/h            # H.264 XU 컨트롤
│   ├── v4l2uvc.c/h                  # V4L2 UVC 드라이버
│   ├── nalu.c/h                     # NAL 유닛 처리
│   ├── cap_desc.c/h                 # 카메라 디스크립터
│   ├── cap_desc_parser.c/h          # 디스크립터 파서
│   └── H264_UVC_TestAP.c            # 테스트 애플리케이션
└── UCAM-Motion_detection/           # 모션 감지 기능
    ├── rervision_xu_ctrls.c/h       # 모션 감지 XU 컨트롤
    ├── v4l2uvc.c/h                  # V4L2 UVC 드라이버
    ├── main.c                       # 모션 감지 메인
    └── uvcvideo.h                   # UVC 비디오 헤더
```

### **SDK 기능**
- **H.264 하드웨어 인코딩**: 카메라 내장 H.264 인코더 제어
- **XU (Extension Unit) 제어**: USB Extension Unit을 통한 고급 기능
- **모션 감지**: 하드웨어 모션 감지 지원
- **V4L2 네이티브**: Linux Video4Linux2 API 직접 사용

## 🚀 **빠른 시작**

### 1. 라즈베리파이에서 의존성 설치
```bash
sudo apt update
sudo apt install build-essential libx11-dev libxext-dev
```

### 2. SDK 의존성 확인
```bash
make -f Makefile_raspberry_pi check-sdk
```

### 3. 빌드
```bash
make -f Makefile_raspberry_pi all
```

### 4. 실행
```bash
./raspberry_pi_viewer -d /dev/video0 -w 1280 -h 720 -f 30
```

## 📋 **라즈베리파이 특화 기능**

### 🍓 **완전한 Linux SDK 지원**
- ✅ **H.264 하드웨어 인코딩**: 카메라 내장 H.264 인코더 활용
- ✅ **XU (Extension Unit) 제어**: 고급 카메라 기능 제어
- ✅ **V4L2 네이티브**: Linux Video4Linux2 API 직접 사용
- ✅ **모션 감지**: 하드웨어 모션 감지 지원
- ✅ **OSD 오버레이**: 화면에 정보 표시
- ✅ **정밀 FPS 제어**: 나노초 단위 제어

### 🏗️ **라즈베리파이 최적화**
- ✅ **ARM 아키텍처 최적화**: ARM32/ARM64 네이티브 지원
- ✅ **실시간 스트리밍**: 낮은 지연시간
- ✅ **메모리 최적화**: 라즈베리파이 메모리 제약 고려
- ✅ **CPU 최적화**: ARM 프로세서 최적화

## 🔧 **설치 및 빌드**

### 라즈베리파이에서만
```bash
# 의존성 설치
sudo apt update
sudo apt install build-essential libx11-dev libxext-dev

# SDK 의존성 확인
make -f Makefile_raspberry_pi check-sdk

# 빌드
make -f Makefile_raspberry_pi all

# 설치 (선택적)
sudo make -f Makefile_raspberry_pi install
```

## 📖 **사용법**

### 기본 사용법
```bash
# 기본 설정으로 실행
./raspberry_pi_viewer

# 특정 장치와 해상도로 실행
./raspberry_pi_viewer -d /dev/video0 -w 1280 -h 720 -f 30

# H.264 포맷으로 실행
./raspberry_pi_viewer -d /dev/video0 -w 1920 -h 1080 -f 60 -F 0x00000021
```

### 명령행 옵션

| 옵션 | 설명 | 기본값 |
|------|------|--------|
| `-d <device>` | 카메라 장치 | `/dev/video0` |
| `-w <width>` | 해상도 너비 | `640` |
| `-h <height>` | 해상도 높이 | `480` |
| `-f <fps>` | FPS | `30` |
| `-b <bitrate>` | 비트레이트 | `1000000` |
| `-q <quality>` | 품질 | `80` |
| `-F <format>` | 포맷 | `0x00000021` (H.264) |

### 지원 포맷

| 포맷 코드 | 설명 | 지원도 |
|-----------|------|--------|
| `0x00000021` | H.264 (하드웨어 인코딩) | ✅ |
| `0x47504A4D` | MJPEG | ✅ |
| `0x56595559` | YUYV | ✅ |
| `0x32315659` | YV12 | ✅ |

## 🎮 **제어**

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

## 🔍 **문제 해결**

### 일반적인 문제

#### 1. 카메라 권한 문제
```bash
sudo usermod -a -G video $USER
# 재로그인 필요
```

#### 2. X11 디스플레이 문제
```bash
export DISPLAY=:0
xhost +local:
```

#### 3. SDK 경로 문제
```bash
make -f Makefile_raspberry_pi check-sdk
# SDK_PATH 변수 확인
```

### 디버깅

```bash
# 상세 출력으로 실행
./raspberry_pi_viewer -v

# 카메라 정보 확인
v4l2-ctl --list-devices
v4l2-ctl -d /dev/video0 --list-formats-ext

# 플랫폼 정보 확인
make -f Makefile_raspberry_pi info
```

## 📊 **성능 비교**

| 항목 | reference.cpp | Raspberry Pi Viewer |
|------|---------------|-------------------|
| **FPS 정밀도** | 밀리초 | **나노초** |
| **하드웨어 가속** | 없음 | **H.264 인코더** |
| **메모리 사용량** | 높음 | **최적화됨** |
| **CPU 사용률** | 높음 | **낮음** |
| **기능 확장성** | 제한적 | **무제한** |
| **라즈베리파이 최적화** | 없음 | **완전 최적화** |

## 🏗️ **아키텍처**

### 클래스 구조
```
RaspberryPiViewer
├── 초기화 및 설정
│   ├── initialize()
│   ├── openCamera()
│   └── setFormat()
├── FPS 제어
│   ├── setTargetFPS()
│   ├── updateFPSControl()
│   └── waitForNextFrame()
├── 스트리밍 제어
│   ├── startStreaming()
│   └── stopStreaming()
├── 프레임 처리
│   ├── captureFrame()
│   └── displayFrame()
└── GUI 관련
    ├── initializeX11Display()
    ├── createWindow()
    └── updateDisplay()
```

### 라즈베리파이 특화 구현
- **V4L2 + X11**: 라즈베리파이 네이티브 지원
- **ARM 최적화**: ARM32/ARM64 아키텍처 최적화
- **Linux SDK**: ELP H.264 USB Camera SDK 완전 지원

## 📝 **개발 정보**

### 빌드 시스템
- **Makefile**: 라즈베리파이 전용 최적화
- **ARM 최적화**: ARM 아키텍처 최적화 플래그
- **조건부 컴파일**: 라즈베리파이 전용 기능 활성화
- **로컬 SDK**: 포함된 SDK 의존성 사용

### 의존성
- **pthread**: 멀티스레딩 지원
- **X11**: GUI 디스플레이
- **Linux SDK**: ELP H.264 USB Camera SDK (로컬 포함)

## ⚠️ **중요 사항**

### 플랫폼 제한
- **라즈베리파이에서만 작동**: macOS나 일반 Linux에서는 빌드되지 않음
- **ARM 아키텍처 전용**: x86_64에서는 작동하지 않음
- **Linux SDK 의존**: ELP Linux SDK가 필요함 (로컬 포함)

### 권장 하드웨어
- **Raspberry Pi 4**: 권장 (ARM64, 4GB RAM 이상)
- **Raspberry Pi 3**: 지원 (ARM32/ARM64)
- **ELP H.264 USB Camera**: 완전한 기능을 위해 권장

## 📄 **라이센스**

이 프로젝트는 ELP Linux SDK를 기반으로 하며, 해당 SDK의 라이센스 조건을 따릅니다.

## 🤝 **기여**

버그 리포트나 기능 요청은 이슈로 등록해 주세요.

---

**참고**: 
- 이 뷰어는 **라즈베리파이에서만 작동**합니다.
- ELP H.264 USB 카메라와 함께 사용하면 최고의 성능을 얻을 수 있습니다.
- 다른 플랫폼에서는 빌드되지 않습니다.
- **SDK 의존성은 로컬에 포함**되어 있어 별도 다운로드가 필요하지 않습니다.
