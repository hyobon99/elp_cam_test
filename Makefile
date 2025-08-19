# OpenCV 웹캠 뷰어 크로스 플랫폼 Makefile
# 지원 플랫폼: Linux, macOS, Raspberry Pi (ARM)

# 플랫폼 감지
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# 기본 설정
CXX = g++
CXXFLAGS = -g -std=c++11 -Wall -Wextra
LDFLAGS = 

# OpenCV 설정 (플랫폼별)
ifeq ($(UNAME_S),Darwin)
    # macOS
    PKG_OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv 2>/dev/null)
    PKG_OPENCV_LIBS := $(shell pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv 2>/dev/null)
    PLATFORM = macos
    CXXFLAGS += -DMACOS
else ifeq ($(UNAME_S),Linux)
    ifeq ($(UNAME_M),aarch64)
        # Raspberry Pi 4 (ARM64)
        PKG_OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv 2>/dev/null)
        PKG_OPENCV_LIBS := $(shell pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv 2>/dev/null)
        PLATFORM = raspberry_pi
        CXXFLAGS += -DRASPBERRY_PI -march=native -mfpu=neon
    else ifeq ($(UNAME_M),armv7l)
        # Raspberry Pi 3 (ARM32)
        PKG_OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv 2>/dev/null)
        PKG_OPENCV_LIBS := $(shell pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv 2>/dev/null)
        PLATFORM = raspberry_pi
        CXXFLAGS += -DRASPBERRY_PI -march=native -mfpu=neon
    else
        # 일반 Linux (x86_64)
        PKG_OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv 2>/dev/null)
        PKG_OPENCV_LIBS := $(shell pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv 2>/dev/null)
        PLATFORM = linux
        CXXFLAGS += -DLINUX
    endif
else
    # 기타 플랫폼
    PKG_OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv 2>/dev/null)
    PKG_OPENCV_LIBS := $(shell pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv 2>/dev/null)
    PLATFORM = unknown
endif

# OpenCV 라이브러리 설정
CXXFLAGS += $(PKG_OPENCV_CFLAGS)
LDFLAGS += $(PKG_OPENCV_LIBS)

# 타겟들
TARGETS = webcam_viewer_basic webcam_viewer_enhanced webcam_viewer_directshow webcam_viewer_sdk

# 소스 파일들
SOURCES = webcam_viewer.cpp webcam_viewer_enhanced.cpp webcam_viewer_directshow.cpp webcam_viewer_sdk.cpp

# 기본 타겟
all: $(TARGETS)

# 개별 타겟 빌드
webcam_viewer_basic: webcam_viewer.cpp
	@echo "빌드 중: $@ (플랫폼: $(PLATFORM))"
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

webcam_viewer_enhanced: webcam_viewer_enhanced.cpp
	@echo "빌드 중: $@ (플랫폼: $(PLATFORM))"
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

webcam_viewer_directshow: webcam_viewer_directshow.cpp
	@echo "빌드 중: $@ (플랫폼: $(PLATFORM))"
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

webcam_viewer_sdk: webcam_viewer_sdk.cpp
	@echo "빌드 중: $@ (플랫폼: $(PLATFORM))"
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# 플랫폼 정보 출력
info:
	@echo "=== 플랫폼 정보 ==="
	@echo "OS: $(UNAME_S)"
	@echo "Architecture: $(UNAME_M)"
	@echo "Platform: $(PLATFORM)"
	@echo "C++ Compiler: $(CXX)"
	@echo "OpenCV CFLAGS: $(PKG_OPENCV_CFLAGS)"
	@echo "OpenCV LIBS: $(PKG_OPENCV_LIBS)"

# OpenCV 설치 확인
check-opencv:
	@echo "=== OpenCV 설치 확인 ==="
	@pkg-config --version opencv4 2>/dev/null && echo "OpenCV4 발견" || echo "OpenCV4 없음"
	@pkg-config --version opencv 2>/dev/null && echo "OpenCV 발견" || echo "OpenCV 없음"
	@echo "OpenCV 라이브러리 경로:"
	@pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv 2>/dev/null || echo "OpenCV 라이브러리를 찾을 수 없습니다"

# 테스트 실행
test: all
	@echo "=== 테스트 실행 ==="
	@echo "기본 뷰어 테스트:"
	@./webcam_viewer_basic --help 2>/dev/null || echo "기본 뷰어 실행 실패"
	@echo "향상된 뷰어 테스트:"
	@./webcam_viewer_enhanced --help 2>/dev/null || echo "향상된 뷰어 실행 실패"

# 정리
clean:
	@echo "빌드 파일 정리 중..."
	rm -f $(TARGETS)
	rm -f *.o
	rm -f *.exe

# 설치 (선택적)
install: all
	@echo "시스템에 설치 중..."
	sudo cp $(TARGETS) /usr/local/bin/
	@echo "설치 완료! 다음 명령어로 실행 가능:"
	@echo "  webcam_viewer_basic"
	@echo "  webcam_viewer_enhanced"
	@echo "  webcam_viewer_directshow"
	@echo "  webcam_viewer_sdk"

# 제거
uninstall:
	@echo "시스템에서 제거 중..."
	sudo rm -f /usr/local/bin/webcam_viewer_*
	@echo "제거 완료!"

# 도움말
help:
	@echo "=== 사용법 ==="
	@echo "make all          - 모든 타겟 빌드"
	@echo "make info         - 플랫폼 정보 출력"
	@echo "make check-opencv - OpenCV 설치 확인"
	@echo "make test         - 빌드된 프로그램 테스트"
	@echo "make clean        - 빌드 파일 정리"
	@echo "make install      - 시스템에 설치"
	@echo "make uninstall    - 시스템에서 제거"
	@echo ""
	@echo "개별 빌드:"
	@echo "make webcam_viewer_basic      - 기본 OpenCV 뷰어"
	@echo "make webcam_viewer_enhanced   - 향상된 뷰어 (FPS 제어)"
	@echo "make webcam_viewer_directshow - DirectShow 통합 뷰어"
	@echo "make webcam_viewer_sdk        - SDK 패턴 활용 뷰어"

.PHONY: all info check-opencv test clean install uninstall help
