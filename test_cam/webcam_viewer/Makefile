# OpenCV 웹캠 뷰어 Makefile

# OpenCV 설정
PKG_OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv 2>/dev/null)
PKG_OPENCV_LIBS := $(shell pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv 2>/dev/null)

CXX = g++
CXXFLAGS = -g -std=c++11 $(PKG_OPENCV_CFLAGS)
LDFLAGS = $(PKG_OPENCV_LIBS)

TARGET = webcam_viewer
SOURCE = opencv_viewer.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall 