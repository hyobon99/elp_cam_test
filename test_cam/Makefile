PKG_SDL2_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
PKG_SDL2_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null)
HAVE_SDL2 := $(shell pkg-config --exists sdl2 && echo yes || echo no)

# OpenCV 설정
PKG_OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv 2>/dev/null)
PKG_OPENCV_LIBS := $(shell pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv 2>/dev/null)
HAVE_OPENCV := $(shell pkg-config --exists opencv4 2>/dev/null && echo yes || pkg-config --exists opencv 2>/dev/null && echo yes || echo no)

CFLAGS = -g -I/usr/src/linux-$(shell uname -r)/include $(PKG_SDL2_CFLAGS) $(PKG_OPENCV_CFLAGS)
LDFLAGS = $(PKG_SDL2_LIBS) $(PKG_OPENCV_LIBS)

CAM_OBJS = ../Linux_UVC_TestAP/v4l2uvc.o

all: $(if $(filter yes,$(HAVE_SDL2)),test_cam,test_cam_pipe) test_cam_mjpeg simple_viewer x11_viewer simple_x11_viewer $(if $(filter yes,$(HAVE_OPENCV)),opencv_viewer,)

../Linux_UVC_TestAP/v4l2uvc.o: ../Linux_UVC_TestAP/v4l2uvc.c ../Linux_UVC_TestAP/v4l2uvc.h ../Linux_UVC_TestAP/debug.h
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c ../Linux_UVC_TestAP/v4l2uvc.h ../Linux_UVC_TestAP/debug.h
	$(CC) $(CFLAGS) -c -o $@ $<

test_cam: main.o $(CAM_OBJS)
	$(CC) $(CFLAGS) main.o $(CAM_OBJS) -o $@ $(LDFLAGS)

main_pipe.o: main_pipe.c ../Linux_UVC_TestAP/v4l2uvc.h ../Linux_UVC_TestAP/debug.h
	$(CC) $(CFLAGS) -c -o $@ $<

test_cam_pipe: main_pipe.o $(CAM_OBJS)
	$(CC) $(CFLAGS) main_pipe.o $(CAM_OBJS) -o $@

main_mjpeg.o: main_mjpeg.c ../Linux_UVC_TestAP/v4l2uvc.h ../Linux_UVC_TestAP/debug.h
	$(CC) $(CFLAGS) -c -o $@ $<

test_cam_mjpeg: main_mjpeg.o $(CAM_OBJS)
	$(CC) $(CFLAGS) main_mjpeg.o $(CAM_OBJS) -o $@

simple_viewer.o: simple_viewer.c ../Linux_UVC_TestAP/v4l2uvc.h ../Linux_UVC_TestAP/debug.h
	$(CC) $(CFLAGS) -c -o $@ $<

simple_viewer: simple_viewer.o $(CAM_OBJS)
	$(CC) $(CFLAGS) simple_viewer.o $(CAM_OBJS) -o $@ $(LDFLAGS)

x11_viewer.o: x11_viewer.c ../Linux_UVC_TestAP/v4l2uvc.h ../Linux_UVC_TestAP/debug.h
	$(CC) $(CFLAGS) -c -o $@ $<

x11_viewer: x11_viewer.o $(CAM_OBJS)
	$(CC) $(CFLAGS) x11_viewer.o $(CAM_OBJS) -o $@ -lX11

simple_x11_viewer.o: simple_x11_viewer.c ../Linux_UVC_TestAP/v4l2uvc.h ../Linux_UVC_TestAP/debug.h
	$(CC) $(CFLAGS) -c -o $@ $<

simple_x11_viewer: simple_x11_viewer.o $(CAM_OBJS)
	$(CC) $(CFLAGS) simple_x11_viewer.o $(CAM_OBJS) -o $@ -lX11

opencv_viewer: opencv_viewer.cpp
	$(CXX) $(CFLAGS) opencv_viewer.cpp -o $@ $(LDFLAGS)

clean:
	-rm -f *.o test_cam test_cam_pipe test_cam_mjpeg simple_viewer x11_viewer simple_x11_viewer opencv_viewer ../Linux_UVC_TestAP/v4l2uvc.o

.PHONY: all clean 