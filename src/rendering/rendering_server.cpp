#include <cstdint>
#include <cstring>

#include "rendering_device.h"
#include "rendering_server.h"

void RS::initialize(int argc, char **argv, const char **extensions, uint32_t extensionCount) {
	bool validation = false;
	for (int i = 0; i < argc; i++) {
		if (strcmp("--validate", argv[i]) == 0)
			validation = true;
	}

	m_renderingDevice = new RenderingDevice;
	m_renderingDevice->create(extensions, extensionCount, validation);
}

VkInstance RS::vulkanInstance() {
	return m_renderingDevice->instance();
}

void RS::spriteCreate(Image *image) {
	m_renderingDevice->spriteCreate(image);
}

void RS::windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
	m_renderingDevice->windowCreate(surface, width, height);
}

void RS::windowResize(uint32_t width, uint32_t height) {
	m_renderingDevice->windowResize(width, height);
}

void RS::draw() {
	m_renderingDevice->draw();
}
