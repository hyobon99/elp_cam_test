# Cross-Platform Linux SDK Professional Viewer

ELP H.264 USB Camera SDK를 기반으로 한 전문적인 크로스 플랫폼 카메라 뷰어입니다.

## 🎯 **지원 플랫폼**

| 플랫폼 | 지원도 | 주요 기능 | 제한사항 |
|--------|--------|-----------|----------|
| **Linux** | ✅ 완전 지원 | 모든 기능 | 없음 |
| **Raspberry Pi** | ✅ 완전 지원 | 모든 기능 | 없음 |
| **macOS** | ⚠️ 제한적 지원 | 기본 기능 | Linux SDK 기능 제한 |

## 🚀 **빠른 시작**

### 1. 호환성 체크
```bash
./check_compatibility.sh
```

### 2. 빌드
```bash
make all
```

### 3. 실행
```bash
./linux_sdk_viewer -d /dev/video0 -w 1280 -h 720 -f 30
```

## 📋 **플랫폼별 특징**

### 🐧 **Linux (완전 지원)**
- ✅ **H.264 하드웨어 인코딩**: 카메라 내장 H.264 인코더 활용
- ✅ **XU (Extension Unit) 제어**: 고급 카메라 기능 제어
- ✅ **V4L2 네이티브**: Linux Video4Linux2 API 직접 사용
- ✅ **모션 감지**: 하드웨어 모션 감지 지원
- ✅ **OSD 오버레이**: 화면에 정보 표시
- ✅ **정밀 FPS 제어**: 나노초 단위 제어

### 🍓 **Raspberry Pi (완전 지원)**
- ✅ **모든 Linux 기능**: Linux와 동일한 기능
- ✅ **ARM 최적화**: ARM 아키텍처 최적화
- ✅ **실시간 스트리밍**: 낮은 지연시간
- ✅ **하드웨어 가속**: H.264 하드웨어 인코딩

### 🍎 **macOS (제한적 지원)**
- ✅ **기본 카메라 기능**: FPS 제어, 해상도 설정
- ✅ **AVFoundation**: macOS 네이티브 카메라 API
- ⚠️ **Linux SDK 기능 제한**: H.264 제어, XU 제어 불가
- ⚠️ **개발/테스트용**: 프로덕션에는 Linux 권장

## 🔧 **설치 및 빌드**

### Linux/Raspberry Pi
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential libx11-dev libxext-dev

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install libX11-devel libXext-devel

# 빌드
make all
```

### macOS
```bash
# Xcode Command Line Tools 설치
xcode-select --install

# 빌드
make all
```

## 📖 **사용법**

### 기본 사용법
```bash
# 기본 설정으로 실행
./linux_sdk_viewer

# 특정 장치와 해상도로 실행
./linux_sdk_viewer -d /dev/video0 -w 1280 -h 720 -f 30

# H.264 포맷으로 실행 (Linux/Raspberry Pi에서만)
./linux_sdk_viewer -d /dev/video0 -w 1920 -h 1080 -f 60 -F 0x00000021
```

### 명령행 옵션

| 옵션 | 설명 | 기본값 | 플랫폼 |
|------|------|--------|--------|
| `-d <device>` | 카메라 장치 | `/dev/video0` | Linux/RPi |
| `-w <width>` | 해상도 너비 | `640` | 모든 플랫폼 |
| `-h <height>` | 해상도 높이 | `480` | 모든 플랫폼 |
| `-f <fps>` | FPS | `30` | 모든 플랫폼 |
| `-b <bitrate>` | 비트레이트 | `1000000` | Linux/RPi |
| `-q <quality>` | 품질 | `80` | Linux/RPi |
| `-F <format>` | 포맷 | `0x00000021` | Linux/RPi |

### 지원 포맷 (Linux/Raspberry Pi)

| 포맷 코드 | 설명 | 지원도 |
|-----------|------|--------|
| `0x00000021` | H.264 (하드웨어 인코딩) | ✅ |
| `0x47504A4D` | MJPEG | ✅ |
| `0x56595559` | YUYV | ✅ |
| `0x32315659` | YV12 | ✅ |

## 🎮 **제어**

### 키보드 제어
| 키 | 기능 | 플랫폼 |
|----|------|--------|
| `ESC` | 종료 | 모든 플랫폼 |
| `R` | 통계 리셋 | 모든 플랫폼 |
| `I` | 카메라 정보 출력 | 모든 플랫폼 |
| `F` | 지원 포맷 목록 | Linux/RPi |

### 시그널 제어
```bash
# Ctrl+C로 안전한 종료
# SIGTERM으로 종료
kill -TERM <pid>
```

## 🔍 **문제 해결**

### 일반적인 문제

#### 1. 카메라 권한 문제 (Linux/Raspberry Pi)
```bash
sudo usermod -a -G video $USER
# 재로그인 필요
```

#### 2. X11 디스플레이 문제 (Linux/Raspberry Pi)
```bash
export DISPLAY=:0
xhost +local:
```

#### 3. SDK 경로 문제 (Linux/Raspberry Pi)
```bash
make check-sdk
# SDK_PATH 변수 확인
```

#### 4. macOS 카메라 권한
- 시스템 환경설정 > 보안 및 개인 정보 보호 > 카메라
- 터미널 앱에 카메라 권한 부여

### 디버깅

```bash
# 상세 출력으로 실행
./linux_sdk_viewer -v

# 카메라 정보 확인 (Linux/RPi)
v4l2-ctl --list-devices
v4l2-ctl -d /dev/video0 --list-formats-ext

# 플랫폼 정보 확인
make info
```

## 📊 **성능 비교**

| 항목 | reference.cpp | Linux SDK Viewer |
|------|---------------|------------------|
| **FPS 정밀도** | 밀리초 | **나노초** |
| **하드웨어 가속** | 없음 | **H.264 인코더** |
| **메모리 사용량** | 높음 | **최적화됨** |
| **CPU 사용률** | 높음 | **낮음** |
| **기능 확장성** | 제한적 | **무제한** |
| **플랫폼 지원** | 제한적 | **크로스 플랫폼** |

## 🏗️ **아키텍처**

### 클래스 구조
```
LinuxSDKViewer
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
    ├── initializeX11Display() (Linux)
    ├── initializeMacOSDisplay() (macOS)
    └── updateDisplay()
```

### 플랫폼별 구현
- **Linux/Raspberry Pi**: V4L2 + X11 + Linux SDK
- **macOS**: AVFoundation + Cocoa

## 📝 **개발 정보**

### 빌드 시스템
- **Makefile**: 크로스 플랫폼 자동 감지
- **플랫폼별 최적화**: ARM, x86_64, Apple Silicon 지원
- **조건부 컴파일**: 플랫폼별 기능 활성화/비활성화

### 의존성
- **Linux/Raspberry Pi**: pthread, X11, Linux SDK
- **macOS**: pthread, AVFoundation, CoreMedia, CoreVideo, Cocoa

## 📄 **라이센스**

이 프로젝트는 ELP Linux SDK를 기반으로 하며, 해당 SDK의 라이센스 조건을 따릅니다.

## 🤝 **기여**

버그 리포트나 기능 요청은 이슈로 등록해 주세요.

---

**참고**: 
- 이 뷰어는 ELP H.264 USB 카메라용으로 설계되었습니다.
- macOS에서는 기본 카메라 기능만 사용 가능합니다.
- Linux SDK 기능을 사용하려면 Linux 또는 Raspberry Pi를 사용하세요.
