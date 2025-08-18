#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <linux/videodev2.h>

#include <SDL2/SDL.h>

#include "../Linux_UVC_TestAP/v4l2uvc.h"
#include "../Linux_UVC_TestAP/debug.h"

int Dbg_Param = TESTAP_DBG_ERR;

static volatile int keep_running = 1;

static void handle_sigint(int sig) {
	(void)sig;
	keep_running = 0;
}

int main(int argc, char** argv) {
	const char* device = "/dev/video0";
	int width = 640;
	int height = 480;
	unsigned int pixfmt = V4L2_PIX_FMT_YUYV; // safer default

	if (argc >= 2) device = argv[1];
	if (argc >= 3) width = atoi(argv[2]);
	if (argc >= 4) height = atoi(argv[3]);

	Dbg_Param = TESTAP_DBG_ERR; // errors only

	struct vdIn cam;
	memset(&cam, 0, sizeof(cam)); // Initialize to zero
	
	if (init_videoIn(&cam, (char*)device, width, height, pixfmt, 1) < 0) {
		fprintf(stderr, "Failed to init camera %s\n", device);
		return 1;
	}

	// Use actual camera resolution
	width = cam.width;
	height = cam.height;
	fprintf(stderr, "Camera initialized: %dx%d\n", width, height);

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
		close_v4l2(&cam);
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("test_cam", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
	if (!window) {
		fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
		SDL_Quit();
		close_v4l2(&cam);
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		renderer = SDL_CreateRenderer(window, -1, 0);
	}
	if (!renderer) {
		fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		close_v4l2(&cam);
		return 1;
	}

	// For YUYV -> use SDL_PIXELFORMAT_YUY2 texture
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YUY2, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!texture) {
		fprintf(stderr, "SDL_CreateTexture error: %s\n", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		close_v4l2(&cam);
		return 1;
	}

	signal(SIGINT, handle_sigint);

	while (keep_running) {
		if (uvcGrab(&cam) < 0) {
			fprintf(stderr, "uvcGrab failed\n");
			break;
		}

		// Only process if we have valid data
		if (cam.framebuffer && cam.framesizeIn > 0) {
			// cam.framebuffer contains YUYV packed data sized framesizeIn
			void* pixels = NULL;
			int pitch = 0;
			if (SDL_LockTexture(texture, NULL, &pixels, &pitch) == 0) {
				int copy_bytes = cam.framesizeIn;
				int expected_pitch = width * 2; // YUYV 16bpp
				if (pitch == expected_pitch) {
					memcpy(pixels, cam.framebuffer, copy_bytes);
				} else {
					int rows = height;
					int row_bytes = expected_pitch;
					uint8_t* dst = (uint8_t*)pixels;
					uint8_t* src = (uint8_t*)cam.framebuffer;
					for (int y = 0; y < rows; ++y) {
						memcpy(dst + y * pitch, src + y * row_bytes, row_bytes);
					}
				}
				SDL_UnlockTexture(texture);
			}

			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) keep_running = 0;
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) keep_running = 0;
		}
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	close_v4l2(&cam);
	return 0;
} 