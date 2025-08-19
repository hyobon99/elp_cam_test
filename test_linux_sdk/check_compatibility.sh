#!/bin/bash

# Cross-Platform Linux SDK Viewer 호환성 체크 스크립트

echo "=== Cross-Platform Linux SDK Viewer 호환성 체크 ==="
echo ""

# 플랫폼 감지
UNAME_S=$(uname -s)
UNAME_M=$(uname -m)

echo "📋 시스템 정보:"
echo "  OS: $UNAME_S"
echo "  Architecture: $UNAME_M"
echo ""

# 플랫폼별 호환성 체크
case "$UNAME_S" in
    "Darwin")
        echo "🍎 macOS 호환성 체크:"
        echo "  ✓ 기본 지원됨"
        echo "  ⚠️  Linux SDK 기능 제한 (AVFoundation 사용)"
        echo "  ✓ FPS 제어 기능 사용 가능"
        echo "  ✓ 해상도 설정 기능 사용 가능"
        echo "  ✗ H.264 하드웨어 인코딩 제어 불가"
        echo "  ✗ XU (Extension Unit) 제어 불가"
        echo "  ✗ 모션 감지 기능 불가"
        echo ""
        
        # macOS 의존성 체크
        echo "🔧 macOS 의존성 체크:"
        if command -v gcc &> /dev/null; then
            echo "  ✓ GCC 설치됨"
        else
            echo "  ✗ GCC 필요 (Xcode Command Line Tools 설치)"
        fi
        
        if command -v g++ &> /dev/null; then
            echo "  ✓ G++ 설치됨"
        else
            echo "  ✗ G++ 필요 (Xcode Command Line Tools 설치)"
        fi
        
        # AVFoundation 프레임워크 체크
        if [ -d "/System/Library/Frameworks/AVFoundation.framework" ]; then
            echo "  ✓ AVFoundation 프레임워크 사용 가능"
        else
            echo "  ✗ AVFoundation 프레임워크 없음"
        fi
        
        echo ""
        echo "📝 macOS 설치 방법:"
        echo "  xcode-select --install"
        echo ""
        ;;
        
    "Linux")
        if [[ "$UNAME_M" == "aarch64" || "$UNAME_M" == "armv7l" ]]; then
            echo "🍓 Raspberry Pi 호환성 체크:"
            echo "  ✓ 완전 지원됨"
            echo "  ✓ Linux SDK 모든 기능 사용 가능"
            echo "  ✓ H.264 하드웨어 인코딩 제어 가능"
            echo "  ✓ XU (Extension Unit) 제어 가능"
            echo "  ✓ 모션 감지 기능 사용 가능"
            echo "  ✓ V4L2 네이티브 지원"
            echo ""
            
            echo "🔧 Raspberry Pi 의존성 체크:"
        else
            echo "🐧 Linux 호환성 체크:"
            echo "  ✓ 완전 지원됨"
            echo "  ✓ Linux SDK 모든 기능 사용 가능"
            echo "  ✓ H.264 하드웨어 인코딩 제어 가능"
            echo "  ✓ XU (Extension Unit) 제어 가능"
            echo "  ✓ 모션 감지 기능 사용 가능"
            echo "  ✓ V4L2 네이티브 지원"
            echo ""
            
            echo "🔧 Linux 의존성 체크:"
        fi
        
        # Linux/Raspberry Pi 의존성 체크
        if command -v gcc &> /dev/null; then
            echo "  ✓ GCC 설치됨"
        else
            echo "  ✗ GCC 필요"
        fi
        
        if command -v g++ &> /dev/null; then
            echo "  ✓ G++ 설치됨"
        else
            echo "  ✗ G++ 필요"
        fi
        
        if pkg-config --exists x11 &> /dev/null; then
            echo "  ✓ X11 개발 라이브러리 설치됨"
        else
            echo "  ✗ X11 개발 라이브러리 필요"
        fi
        
        if [ -d "/usr/include/linux" ]; then
            echo "  ✓ Linux 헤더 파일 설치됨"
        else
            echo "  ✗ Linux 헤더 파일 필요"
        fi
        
        # V4L2 체크
        if [ -e "/dev/video0" ]; then
            echo "  ✓ V4L2 장치 발견"
        else
            echo "  ⚠️  V4L2 장치 없음 (카메라 연결 확인)"
        fi
        
        echo ""
        
        if [[ "$UNAME_M" == "aarch64" || "$UNAME_M" == "armv7l" ]]; then
            echo "📝 Raspberry Pi 설치 방법:"
            echo "  sudo apt update"
            echo "  sudo apt install build-essential libx11-dev libxext-dev"
            echo ""
        else
            echo "📝 Linux 설치 방법:"
            echo "  # Ubuntu/Debian:"
            echo "  sudo apt install build-essential libx11-dev libxext-dev"
            echo ""
            echo "  # CentOS/RHEL:"
            echo "  sudo yum groupinstall \"Development Tools\""
            echo "  sudo yum install libX11-devel libXext-devel"
            echo ""
        fi
        ;;
        
    *)
        echo "❌ 지원되지 않는 플랫폼: $UNAME_S"
        echo "  지원 플랫폼: macOS, Linux, Raspberry Pi"
        echo ""
        ;;
esac

# SDK 경로 체크 (Linux에서만)
if [[ "$UNAME_S" == "Linux" ]]; then
    echo "📦 Linux SDK 경로 체크:"
    SDK_PATH="../LINUX 开发包-2/ELP Linux SDK最新/Linux"
    
    if [ -d "$SDK_PATH" ]; then
        echo "  ✓ SDK 경로 발견: $SDK_PATH"
        echo "  📁 SDK 내용:"
        ls -la "$SDK_PATH" | head -10
        echo ""
    else
        echo "  ✗ SDK 경로 없음: $SDK_PATH"
        echo "  ⚠️  SDK 기능이 제한됩니다"
        echo ""
    fi
fi

# 빌드 테스트
echo "🔨 빌드 테스트:"
if make info &> /dev/null; then
    echo "  ✓ Makefile 정상 작동"
else
    echo "  ✗ Makefile 오류"
fi

echo ""
echo "=== 호환성 체크 완료 ==="
echo ""

# 권장사항
echo "💡 권장사항:"
case "$UNAME_S" in
    "Darwin")
        echo "  • macOS에서는 기본 카메라 기능만 사용 가능"
        echo "  • Linux SDK 기능을 사용하려면 Linux/Raspberry Pi 사용 권장"
        echo "  • 개발/테스트용으로는 적합함"
        ;;
    "Linux")
        if [[ "$UNAME_M" == "aarch64" || "$UNAME_M" == "armv7l" ]]; then
            echo "  • Raspberry Pi에서 모든 기능 사용 가능"
            echo "  • ELP H.264 USB 카메라와 함께 사용 권장"
            echo "  • 실시간 스트리밍에 최적화됨"
        else
            echo "  • Linux에서 모든 기능 사용 가능"
            echo "  • ELP H.264 USB 카메라와 함께 사용 권장"
            echo "  • 개발 및 프로덕션 환경에 적합함"
        fi
        ;;
esac

echo ""
echo "🚀 다음 단계:"
echo "  1. make check    # 의존성 및 SDK 체크"
echo "  2. make all      # 빌드"
echo "  3. ./linux_sdk_viewer  # 실행"
