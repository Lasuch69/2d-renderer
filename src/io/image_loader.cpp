#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <stb/stb_image.h>

#include "image.h"
#include "image_loader.h"

static void debugInfo(uint32_t width, uint32_t height) {
	printf("Image loaded!\n");
	printf("Width: %dpx\n", width);
	printf("Height: %dpx\n", height);
}

static Image *imageCreate(stbi_uc *data, int width, int height, int numChannels) {
	if (data == nullptr) {
		perror("Image failed to load!\n");
		return nullptr;
	}

	uint32_t pixelCount = width * height;
	size_t size = pixelCount * 4;

	if (numChannels == 4) {
		debugInfo(width, height);
		return new Image(width, height, data, size);
	}

	uint8_t *newData = (uint8_t *)malloc(size);
	for (uint32_t pixel = 0; pixel < pixelCount; pixel++) {
		uint8_t channels[4] = { 0, 0, 0, 255 };
		memcpy(channels, &data[pixel * numChannels], numChannels);
		memcpy(&newData[pixel * 4], channels, 4);
	}

	debugInfo(width, height);
	return new Image(width, height, newData, size);
}

Image *imageLoad(const char *filename) {
	int width, height, numChannels;
	stbi_uc *data = stbi_load(filename, &width, &height, &numChannels, STBI_default);
	return imageCreate(data, width, height, numChannels);
}

Image *imageLoadFromMemory(void *buffer, size_t bufferSize) {
	int width, height, numChannels;
	stbi_uc *data = stbi_load_from_memory((stbi_uc *)buffer, bufferSize, &width, &height, &numChannels, STBI_default);
	return imageCreate(data, width, height, numChannels);
}
