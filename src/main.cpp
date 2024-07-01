#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>

#include "io/image_loader.h"
#include "rendering/rendering_server.h"

#define SDL_SUCCESS 0

const int WIDTH = 1280;
const int HEIGHT = 720;

const double TIMESTEP = 1.0 / 30.0;

int main(int argc, char *argv[]) {
	// Force X11; Wayland is a buggy mess in SDL 2.0
	SDL_SetHint(SDL_HINT_VIDEODRIVER, "x11");

	if (SDL_Init(SDL_INIT_VIDEO) != SDL_SUCCESS) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
		return EXIT_FAILURE;
	}

	uint32_t flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
	SDL_Window *window = SDL_CreateWindow("App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, flags);

	if (window == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	uint32_t extensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);

	const char **extensions = (const char **)malloc(extensionCount * sizeof(const char *));
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions);

	RS::singleton().initialize(argc, argv, extensions, extensionCount);
	VkInstance instance = RS::singleton().vulkanInstance();

	VkSurfaceKHR surface;
	SDL_Vulkan_CreateSurface(window, instance, &surface);
	RS::singleton().windowCreate(surface, WIDTH, HEIGHT);

	double accumulator = 0.0;
	double time = 0.0;

	uint64_t lastTick, currentTick;
	lastTick = SDL_GetPerformanceCounter();

	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				quit = true;

			if (event.type == SDL_WINDOWEVENT_RESIZED) {
				int width, height;
				SDL_Vulkan_GetDrawableSize(window, &width, &height);
				RS::singleton().windowResize(width, height);
			}

			if (event.type == SDL_DROPFILE) {
				char *filename = event.drop.file;
				Image *image = imageLoad(filename);

				if (image != nullptr)
					RS::singleton().spriteCreate(image);

				SDL_free(filename);
			}
		}

		currentTick = SDL_GetPerformanceCounter();
		double frameTime = (double)(currentTick - lastTick) / (double)SDL_GetPerformanceFrequency();
		lastTick = currentTick;

		accumulator += frameTime;

		while (accumulator >= TIMESTEP) {
			// game logic
			// printf("Frame time: %lf\n", frameTime);
			// printf("Time: %lf\n", time);

			accumulator -= TIMESTEP;
			time += TIMESTEP;
		}

		// const double fraction = accumulator / TIMESTEP;

		RS::singleton().draw();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}
