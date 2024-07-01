#ifndef RENDERING_DEVICE_H
#define RENDERING_DEVICE_H

#include <cstddef>
#include <cstdint>

#include <vulkan/vulkan_core.h>

#include "types/allocated.h"
#include "types/pipeline.h"

#include "vulkan_context.h"

const uint32_t FRAMES_IN_FLIGHT = 2;

class Image;

typedef struct VmaAllocator_T *VmaAllocator;
typedef struct VmaAllocationInfo VmaAllocationInfo;

typedef struct {
	float projectionMatrix[16];
	float viewMatrix[16];
} SceneUBO;

typedef struct {
	float modelMatrix[16];
} ObjectConstants;

class RenderingDevice {
private:
	VulkanContext m_context;
	bool m_initialized = false;

	uint32_t m_frame = 0;
	uint32_t m_width, m_height;
	bool m_resized = false;

	VmaAllocator m_allocator;

	VkCommandBuffer m_commandBuffers[FRAMES_IN_FLIGHT];
	VkSemaphore m_presentSemaphores[FRAMES_IN_FLIGHT];
	VkSemaphore m_renderSemaphores[FRAMES_IN_FLIGHT];
	VkFence m_renderFences[FRAMES_IN_FLIGHT];

	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_uniformSetLayout;
	VkDescriptorSet m_uniformSets[FRAMES_IN_FLIGHT];

	AllocatedBuffer m_uniformBuffers[FRAMES_IN_FLIGHT];
	VmaAllocationInfo *m_uniformBufferAllocInfos;

	AllocatedImage m_image;
	bool m_imageValid = false;
	uint32_t m_imageWidth, m_imageHeight;
	VkImageView m_imageView;
	VkSampler m_sampler;

	VkDescriptorSetLayout m_textureSetLayout;
	VkDescriptorSet m_textureSet;

	Pipeline m_checkerboardPipeline;
	Pipeline m_spritePipeline;

	VkCommandBuffer _beginSingleTimeCommands();
	void _endSingleTimeCommands(VkCommandBuffer commandBuffer);

	AllocatedBuffer _bufferCreate(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo *allocInfo);
	void _bufferCopy(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size);
	void _bufferUpdate(VkBuffer buffer, void *data, size_t size);
	void _bufferDestroy(AllocatedBuffer buffer);

	AllocatedImage _imageCreate(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
	void _imageUpdate(VkImage image, uint32_t width, uint32_t height, VkFormat format, void *data, size_t size);
	void _imageDestroy(AllocatedImage image);

	VkImageView _imageViewCreate(VkImage image, VkFormat format);
	void _imageViewDestroy(VkImageView imageView);

public:
	VkInstance instance();

	void draw();

	void spriteCreate(Image *image);

	void windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void windowResize(uint32_t width, uint32_t height);

	void create(const char *const *extensions, uint32_t extensionCount, bool validation);
	void destroy();
};

typedef RenderingDevice RD;

#endif // !RENDERING_DEVICE_H
