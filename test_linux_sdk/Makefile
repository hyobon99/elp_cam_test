# Cross-Platform Linux SDK Viewer Makefile
# 지원 플랫폼: macOS, Linux, Raspberry Pi

# 플랫폼 감지
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# 기본 설정
CXX = g++
CC = gcc
CXXFLAGS = -Wall -Wextra -O2 -g -std=c++11
CFLAGS = -Wall -Wextra -O2 -g

# 플랫폼별 설정
ifeq ($(UNAME_S),Darwin)
    # macOS
    PLATFORM = macos
    CXXFLAGS += -DPLATFORM_MACOS
    CFLAGS += -DPLATFORM_MACOS
    LIBS = -lpthread -framework AVFoundation -framework CoreMedia -framework CoreVideo -framework Cocoa
    SDK_SUPPORT = NO
    # macOS에서는 Linux SDK 기능이 제한됩니다 (AVFoundation 사용)
else ifeq ($(UNAME_S),Linux)
    ifeq ($(UNAME_M),aarch64)
        # Raspberry Pi 4 (ARM64)
        PLATFORM = raspberry_pi
        CXXFLAGS += -DPLATFORM_LINUX -DPLATFORM_RASPBERRY_PI -march=native
        CFLAGS += -DPLATFORM_LINUX -DPLATFORM_RASPBERRY_PI -march=native
        LIBS = -lpthread -lX11 -lm
        SDK_SUPPORT = YES
    else ifeq ($(UNAME_M),armv7l)
        # Raspberry Pi 3 (ARM32)
        PLATFORM = raspberry_pi
        CXXFLAGS += -DPLATFORM_LINUX -DPLATFORM_RASPBERRY_PI -march=native
        CFLAGS += -DPLATFORM_LINUX -DPLATFORM_RASPBERRY_PI -march=native
        LIBS = -lpthread -lX11 -lm
        SDK_SUPPORT = YES
    else
        # 일반 Linux (x86_64)
        PLATFORM = linux
        CXXFLAGS += -DPLATFORM_LINUX
        CFLAGS += -DPLATFORM_LINUX
        LIBS = -lpthread -lX11 -lm
        SDK_SUPPORT = YES
    endif
else
    # 기타 플랫폼
    PLATFORM = unknown
    CXXFLAGS += -DPLATFORM_UNKNOWN
    CFLAGS += -DPLATFORM_UNKNOWN
    LIBS = -lpthread
    SDK_SUPPORT = NO
endif

# Linux SDK 경로 (Linux에서만)
ifeq ($(SDK_SUPPORT),YES)
    SDK_PATH = ../LINUX\ 开发包-2/ELP\ Linux\ SDK最新/Linux
    SDK_INCLUDE = -I$(SDK_PATH)/OSD-Linux_H264_AP_0724
    SDK_SOURCES = $(SDK_PATH)/OSD-Linux_H264_AP_0724/h264_xu_ctrls.c \
                  $(SDK_PATH)/OSD-Linux_H264_AP_0724/v4l2uvc.c \
                  $(SDK_PATH)/OSD-Linux_H264_AP_0724/nalu.c \
                  $(SDK_PATH)/OSD-Linux_H264_AP_0724/cap_desc.c \
                  $(SDK_PATH)/OSD-Linux_H264_AP_0724/cap_desc_parser.c
else
    SDK_INCLUDE = 
    SDK_SOURCES = 
endif

# 소스 파일들
SOURCES = main_linux_sdk.cpp linux_sdk_viewer.cpp
OBJECTS = $(SOURCES:.cpp=.o) $(SDK_SOURCES:.c=.o)

# 타겟
TARGET = linux_sdk_viewer

# 기본 타겟
all: $(TARGET)

# 메인 타겟 빌드
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "빌드 완료: $(TARGET) (플랫폼: $(PLATFORM))"

# C++ 소스 컴파일
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(SDK_INCLUDE) -c $< -o $@

# C 소스 컴파일 (SDK 파일들, Linux에서만)
%.o: %.c
ifeq ($(SDK_SUPPORT),YES)
	$(CC) $(CFLAGS) $(SDK_INCLUDE) -c $< -o $@
else
	@echo "SDK 파일 건너뛰기: $< (플랫폼: $(PLATFORM))"
	@touch $@
endif

# 정리
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "정리 완료"

# 설치 (선택적)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "설치 완료: /usr/local/bin/$(TARGET)"

# 제거
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "제거 완료"

# 플랫폼 정보
info:
	@echo "=== 플랫폼 정보 ==="
	@echo "OS: $(UNAME_S)"
	@echo "Architecture: $(UNAME_M)"
	@echo "Platform: $(PLATFORM)"
	@echo "SDK Support: $(SDK_SUPPORT)"
	@echo "C++ Compiler: $(CXX)"
	@echo "C Compiler: $(CC)"
	@echo "Libraries: $(LIBS)"

# 의존성 체크
check-deps:
	@echo "=== 의존성 체크 ==="
	@which $(CC) > /dev/null && echo "✓ GCC 설치됨" || echo "✗ GCC 필요"
	@which $(CXX) > /dev/null && echo "✓ G++ 설치됨" || echo "✗ G++ 필요"
ifeq ($(UNAME_S),Darwin)
	@echo "✓ macOS AVFoundation 프레임워크 사용"
else
	@pkg-config --exists x11 && echo "✓ X11 개발 라이브러리 설치됨" || echo "✗ X11 개발 라이브러리 필요"
endif
	@echo "=================="

# SDK 경로 체크 (Linux에서만)
check-sdk:
ifeq ($(SDK_SUPPORT),YES)
	@echo "=== SDK 경로 체크 ==="
	@if [ -d "$(SDK_PATH)" ]; then \
		echo "✓ SDK 경로 발견: $(SDK_PATH)"; \
		ls -la "$(SDK_PATH)"; \
	else \
		echo "✗ SDK 경로 없음: $(SDK_PATH)"; \
		echo "SDK_PATH 변수를 올바른 경로로 수정하세요"; \
	fi
	@echo "===================="
else
	@echo "=== SDK 경로 체크 ==="
	@echo "현재 플랫폼에서는 SDK가 지원되지 않습니다: $(PLATFORM)"
	@echo "===================="
endif

# 전체 체크
check: check-deps check-sdk
	@echo "=== 전체 체크 완료 ==="

# 도움말
help:
	@echo "=== Cross-Platform Linux SDK Viewer ==="
	@echo "지원 플랫폼: macOS, Linux, Raspberry Pi"
	@echo ""
	@echo "make all        - 빌드"
	@echo "make clean      - 정리"
	@echo "make install    - 시스템 설치"
	@echo "make uninstall  - 시스템 제거"
	@echo "make info       - 플랫폼 정보"
	@echo "make check      - 의존성 및 SDK 체크"
	@echo "make help       - 이 도움말"
	@echo ""
	@echo "=== 플랫폼별 특징 ==="
	@echo "macOS: AVFoundation 기반 (SDK 기능 제한)"
	@echo "Linux: V4L2 + X11 + Linux SDK 완전 지원"
	@echo "Raspberry Pi: V4L2 + X11 + Linux SDK 완전 지원"
	@echo ""
	@echo "=== 사용법 ==="
	@echo "./$(TARGET) [옵션]"
	@echo "  -d <device>   카메라 장치"
	@echo "  -w <width>    해상도 너비"
	@echo "  -h <height>   해상도 높이"
	@echo "  -f <fps>      FPS"
	@echo "  -F <format>   포맷 (Linux에서만)"

.PHONY: all clean install uninstall info check-deps check-sdk check help
