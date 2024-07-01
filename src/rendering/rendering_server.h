#ifndef RENDERING_SERVER_H
#define RENDERING_SERVER_H

#include <cstdint>

class Image;
class RenderingDevice;

typedef struct VkInstance_T *VkInstance;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

class RenderingServer {
public:
	static RenderingServer &singleton() {
		static RenderingServer instance;
		return instance;
	}

	RenderingServer(RenderingServer const &) = delete;
	void operator=(RenderingServer const &) = delete;

private:
	RenderingDevice *m_renderingDevice;

	RenderingServer() {}

public:
	void initialize(int argc, char **argv, const char **extensions, uint32_t extensionCount);

	VkInstance vulkanInstance();

	void spriteCreate(Image *image);

	void windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void windowResize(uint32_t width, uint32_t height);

	void draw();
};

typedef RenderingServer RS;

#endif // !RENDERING_SERVER_H
