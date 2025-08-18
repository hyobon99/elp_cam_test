#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <linux/videodev2.h>

#include "../Linux_UVC_TestAP/v4l2uvc.h"
#include "../Linux_UVC_TestAP/debug.h"

int Dbg_Param = TESTAP_DBG_ERR;

static volatile int keep_running = 1;
static void handle_sigint(int sig) { (void)sig; keep_running = 0; }

static int have_ffplay(void) {
	return system("command -v ffplay >/dev/null 2>&1") == 0;
}

int main(int argc, char** argv) {
	const char* device = "/dev/video0";
	int width = 640;
	int height = 480;
	unsigned int pixfmt = V4L2_PIX_FMT_YUYV;
	int use_ffplay = 1;

	if (argc >= 2) device = argv[1];
	if (argc >= 3) width = atoi(argv[2]);
	if (argc >= 4) height = atoi(argv[3]);
	if (argc >= 5) use_ffplay = atoi(argv[4]);

	Dbg_Param = TESTAP_DBG_ERR;

	struct vdIn cam;
	memset(&cam, 0, sizeof(cam)); // Initialize to zero
	
	if (init_videoIn(&cam, (char*)device, width, height, pixfmt, 1) < 0) {
		fprintf(stderr, "init_videoIn failed\n");
		return 1;
	}

	// Use actual camera resolution
	width = cam.width;
	height = cam.height;
	fprintf(stderr, "Camera initialized: %dx%d\n", width, height);

	signal(SIGINT, handle_sigint);

	int pipefd[2] = {-1,-1};
	pid_t child = -1;
	FILE* out = stdout;

	if (use_ffplay && have_ffplay()) {
		if (pipe(pipefd) == 0) {
			child = fork();
			if (child == 0) {
				// child: ffplay reads from stdin
				dup2(pipefd[0], STDIN_FILENO);
				close(pipefd[0]);
				close(pipefd[1]);
				char size_arg[32];
				snprintf(size_arg, sizeof(size_arg), "%dx%d", width, height);
				execlp("ffplay", "ffplay", "-f", "rawvideo", "-pixel_format", "yuyv422", "-video_size",
					size_arg, "-i", "-", "-autoexit", "-loglevel", "error", (char*)NULL);
				_exit(127);
			}
			// parent
			close(pipefd[0]);
			out = fdopen(pipefd[1], "wb");
		}
	}

	while (keep_running) {
		if (uvcGrab(&cam) < 0) {
			fprintf(stderr, "uvcGrab failed\n");
			break;
		}
		
		// Only write if we have valid data
		if (cam.framebuffer && cam.framesizeIn > 0) {
			fwrite(cam.framebuffer, 1, cam.framesizeIn, out);
			fflush(out);
		}
	}

	if (out && out != stdout) fclose(out);
	if (child > 0) {
		int status=0; 
		waitpid(child, &status, 0);
	}
	close_v4l2(&cam);
	return 0;
} 