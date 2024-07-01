#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "io/image.h"
#include "io/image_loader.h"
#include "math/matrix.h"
#include "rendering/shaders/glsl/checkerboard.gen.h"
#include "rendering/shaders/glsl/sprite.gen.h"

#include "rendering_device.h"

#define CHECK_VK_RESULT(_expr, msg)                                                                                    \
	if (!(_expr)) {                                                                                                    \
		printf("%s\n", msg);                                                                                           \
	}

static VkPipeline pipelineCreate(VkDevice device, VkShaderModule vertexModule, VkShaderModule fragmentModule,
		VkPipelineLayout pipelineLayout, VkRenderPass renderPass, uint32_t subpass) {
	VkPipelineShaderStageCreateInfo vertexStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertexModule,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo fragmentStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragmentModule,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo shaderStageInfos[2] = {
		vertexStageInfo,
		fragmentStageInfo,
	};

	VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	VkPipelineViewportStateCreateInfo viewportStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
	};

	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_FALSE,
		.depthWriteEnable = VK_FALSE,
		.depthCompareOp = VK_COMPARE_OP_NEVER,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
						  VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
	};

	VkDynamicState dynamicStates[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates,
	};

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStageInfos,
		.pVertexInputState = &vertexInputStateInfo,
		.pInputAssemblyState = &inputAssemblyStateInfo,
		.pViewportState = &viewportStateInfo,
		.pRasterizationState = &rasterizationStateInfo,
		.pMultisampleState = &multisampleStateInfo,
		.pDepthStencilState = &depthStencilStateInfo,
		.pColorBlendState = &colorBlendStateInfo,
		.pDynamicState = &dynamicStateInfo,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = subpass,
	};

	VkPipeline pipeline;
	CHECK_VK_RESULT(
			vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline) == VK_SUCCESS,
			"Graphics pipeline creation failed!");

	return pipeline;
}

VkCommandBuffer RD::_beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_context.commandPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_context.device(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void RD::_endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
	};

	vkQueueSubmit(m_context.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_context.graphicsQueue());
	vkFreeCommandBuffers(m_context.device(), m_context.commandPool(), 1, &commandBuffer);
}

AllocatedBuffer RD::_bufferCreate(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo *allocInfo) {
	VkBufferCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
	};

	VmaAllocationCreateInfo allocCreateInfo = {
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};

	AllocatedBuffer buffer;
	vmaCreateBuffer(m_allocator, &createInfo, &allocCreateInfo, &buffer.handle, &buffer.allocation, allocInfo);
	return buffer;
}

void RD::_bufferCopy(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size) {
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

	VkBufferCopy bufferCopy = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size = size,
	};

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

	_endSingleTimeCommands(commandBuffer);
}

void RD::_bufferUpdate(VkBuffer buffer, void *data, size_t size) {
	VmaAllocationInfo allocInfo;
	AllocatedBuffer staging = _bufferCreate(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &allocInfo);

	memcpy(allocInfo.pMappedData, data, size);
	vmaFlushAllocation(m_allocator, staging.allocation, 0, VK_WHOLE_SIZE);
	_bufferCopy(staging.handle, buffer, size);
}

void RD::_bufferDestroy(AllocatedBuffer buffer) {
	vmaDestroyBuffer(m_allocator, buffer.handle, buffer.allocation);
}

AllocatedImage RD::_imageCreate(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage) {
	VkImageCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = { width, height, 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VmaAllocationCreateInfo allocInfo = {
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
		.priority = 1.0f,
	};

	AllocatedImage image;
	vmaCreateImage(m_allocator, &createInfo, &allocInfo, &image.handle, &image.allocation, nullptr);

	return image;
}

void RD::_imageUpdate(VkImage image, uint32_t width, uint32_t height, VkFormat format, void *data, size_t size) {
	VmaAllocationInfo stagingAllocInfo;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	AllocatedBuffer stagingBuffer = _bufferCreate(size, usage, &stagingAllocInfo);
	memcpy(stagingAllocInfo.pMappedData, data, size);
	vmaFlushAllocation(m_allocator, stagingBuffer.allocation, 0, VK_WHOLE_SIZE);

	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

	{
		VkImageSubresourceRange subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		VkImageMemoryBarrier imageBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = subresourceRange,
		};

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
				nullptr, 0, nullptr, 1, &imageBarrier);
	}

	{
		VkImageSubresourceLayers imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		VkExtent3D imageExtent = {
			.width = width,
			.height = height,
			.depth = 1,
		};

		VkBufferImageCopy region = {
			.imageSubresource = imageSubresource,
			.imageExtent = imageExtent,
		};

		vkCmdCopyBufferToImage(
				commandBuffer, stagingBuffer.handle, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	{
		VkImageSubresourceRange subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		VkImageMemoryBarrier imageBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = subresourceRange,
		};

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
				nullptr, 0, nullptr, 1, &imageBarrier);
	}

	_endSingleTimeCommands(commandBuffer);
	_bufferDestroy(stagingBuffer);
}

void RD::_imageDestroy(AllocatedImage image) {
	vmaDestroyImage(m_allocator, image.handle, image.allocation);
}

VkImageView RD::_imageViewCreate(VkImage image, VkFormat format) {
	VkImageSubresourceRange subresourceRange = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	VkImageViewCreateInfo viewInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = subresourceRange,
	};

	VkImageView view;
	CHECK_VK_RESULT(vkCreateImageView(m_context.device(), &viewInfo, nullptr, &view) == VK_SUCCESS,
			"Image view creation failed!");

	return view;
}

void RD::_imageViewDestroy(VkImageView imageView) {
	vkDestroyImageView(m_context.device(), imageView, nullptr);
}

VkInstance RD::instance() {
	return m_context.instance();
}

void RD::draw() {
	CHECK_VK_RESULT(vkWaitForFences(m_context.device(), 1, &m_renderFences[m_frame], VK_TRUE, UINT64_MAX) == VK_SUCCESS,
			"Fence timed out!");

	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(m_context.device(), m_context.swapchain(), UINT64_MAX,
			m_presentSemaphores[m_frame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		m_context.windowResize(m_width, m_height);
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		printf("Swapchain image acquire failed!\n");
	}

	vkResetFences(m_context.device(), 1, &m_renderFences[m_frame]);

	vkResetCommandBuffer(m_commandBuffers[m_frame], 0);

	VkExtent2D extent = m_context.swapchainExtent();
	Matrix projection = projectionMatrix(extent.width, extent.height);
	Matrix view = viewMatrix(0.0, 0.0);

	SceneUBO ubo;
	memcpy(ubo.projectionMatrix, projection.data, sizeof(projection.data));
	memcpy(ubo.viewMatrix, view.data, sizeof(view.data));

	memcpy(m_uniformBufferAllocInfos[m_frame].pMappedData, &ubo, sizeof(ubo));

	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	vkBeginCommandBuffer(m_commandBuffers[m_frame], &beginInfo);

	VkClearValue clearValue = {
		.color = { { 0.0f, 0.0f, 0.0f, 1.0f } },
	};

	VkViewport viewport = {
		.width = (float)extent.width,
		.height = (float)extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor = {
		.extent = extent,
	};

	VkRect2D renderArea = {
		.extent = extent,
	};

	VkRenderPassBeginInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_context.renderPass(),
		.framebuffer = m_context.framebuffer(imageIndex),
		.renderArea = renderArea,
		.clearValueCount = 1,
		.pClearValues = &clearValue,
	};

	vkCmdBeginRenderPass(m_commandBuffers[m_frame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdSetViewport(m_commandBuffers[m_frame], 0, 1, &viewport);
	vkCmdSetScissor(m_commandBuffers[m_frame], 0, 1, &scissor);

	vkCmdBindPipeline(m_commandBuffers[m_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_checkerboardPipeline.handle);
	vkCmdDraw(m_commandBuffers[m_frame], 3, 1, 0, 0);

	if (m_imageValid) {
		vkCmdBindPipeline(m_commandBuffers[m_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_spritePipeline.handle);

		VkDescriptorSet descriptorSets[] = {
			m_uniformSets[m_frame],
			m_textureSet,
		};

		vkCmdBindDescriptorSets(m_commandBuffers[m_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_spritePipeline.layout, 0,
				2, descriptorSets, 0, nullptr);

		Matrix model = modelMatrix(0.0, 0.0, 0.0, m_imageWidth, m_imageHeight);

		ObjectConstants constants;
		memcpy(constants.modelMatrix, model.data, sizeof(model.data));

		vkCmdPushConstants(m_commandBuffers[m_frame], m_spritePipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
				sizeof(constants), &constants);

		vkCmdDraw(m_commandBuffers[m_frame], 6, 1, 0, 0);
	}

	vkCmdEndRenderPass(m_commandBuffers[m_frame]);
	vkEndCommandBuffer(m_commandBuffers[m_frame]);

	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_presentSemaphores[m_frame],
		.pWaitDstStageMask = &waitDstStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_commandBuffers[m_frame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_renderSemaphores[m_frame],
	};

	vkQueueSubmit(m_context.graphicsQueue(), 1, &submitInfo, m_renderFences[m_frame]);

	VkSwapchainKHR swapchain = m_context.swapchain();

	VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderSemaphores[m_frame],
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &imageIndex,
	};

	result = vkQueuePresentKHR(m_context.presentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_resized) {
		m_context.windowResize(m_width, m_height);
		m_resized = false;
	} else if (result != VK_SUCCESS) {
		printf("Swapchain image presentation failed!\n");
	}

	m_frame = (m_frame + 1) % FRAMES_IN_FLIGHT;
}

void RD::spriteCreate(Image *image) {
	vkDeviceWaitIdle(m_context.device());

	if (m_imageValid) {
		vkDestroyImageView(m_context.device(), m_imageView, nullptr);
		vmaDestroyImage(m_allocator, m_image.handle, m_image.allocation);
	}

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	m_image = _imageCreate(image->width(), image->height(), format, usage);
	m_imageView = _imageViewCreate(m_image.handle, format);

	m_imageWidth = image->width();
	m_imageHeight = image->height();

	_imageUpdate(m_image.handle, image->width(), image->height(), format, image->data(), image->size());

	VkDescriptorImageInfo imageInfo = {
		.imageView = m_imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet imageWriteInfo = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_textureSet,
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &imageInfo,
	};

	vkUpdateDescriptorSets(m_context.device(), 1, &imageWriteInfo, 0, nullptr);

	m_imageValid = true;
}

void RD::windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
	m_context.windowCreate(surface, width, height);

	m_width = width;
	m_height = height;

	// allocator

	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
		allocatorInfo.instance = m_context.instance();
		allocatorInfo.physicalDevice = m_context.physicalDevice();
		allocatorInfo.device = m_context.device();

		CHECK_VK_RESULT(vmaCreateAllocator(&allocatorInfo, &m_allocator) == VK_SUCCESS, "Allocator creation failed!");
	}

	// commands

	{
		VkCommandBufferAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_context.commandPool(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = FRAMES_IN_FLIGHT,
		};

		CHECK_VK_RESULT(vkAllocateCommandBuffers(m_context.device(), &allocInfo, m_commandBuffers) == VK_SUCCESS,
				"Command buffers allocation failed!");
	}

	// sync

	{
		VkSemaphoreCreateInfo semaphoreInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		VkFenceCreateInfo fenceInfo = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			vkCreateSemaphore(m_context.device(), &semaphoreInfo, nullptr, &m_presentSemaphores[i]);
			vkCreateSemaphore(m_context.device(), &semaphoreInfo, nullptr, &m_renderSemaphores[i]);
			vkCreateFence(m_context.device(), &fenceInfo, nullptr, &m_renderFences[i]);
		}
	}

	// descriptor pool

	{
		VkDescriptorPoolSize poolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 },
		};

		uint32_t maxSets = 0;
		for (const VkDescriptorPoolSize &poolSize : poolSizes) {
			maxSets += poolSize.descriptorCount;
		}

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = maxSets,
			.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]),
			.pPoolSizes = poolSizes,
		};

		CHECK_VK_RESULT(vkCreateDescriptorPool(m_context.device(), &descriptorPoolCreateInfo, nullptr,
								&m_descriptorPool) == VK_SUCCESS,
				"Descriptor pool creation failed!");
	}

	// uniform buffers

	{
		VkDescriptorSetLayoutBinding binding = {
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		};

		VkDescriptorSetLayoutCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 1,
			.pBindings = &binding,
		};

		CHECK_VK_RESULT(vkCreateDescriptorSetLayout(m_context.device(), &createInfo, nullptr, &m_uniformSetLayout) ==
								VK_SUCCESS,
				"Uniform set layout creation failed!");

		VkDescriptorSetLayout uniformSetLayouts[FRAMES_IN_FLIGHT];
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			uniformSetLayouts[i] = m_uniformSetLayout;
		}

		VkDescriptorSetAllocateInfo uniformSetAllocInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = m_descriptorPool,
			.descriptorSetCount = FRAMES_IN_FLIGHT,
			.pSetLayouts = uniformSetLayouts,
		};

		CHECK_VK_RESULT(vkAllocateDescriptorSets(m_context.device(), &uniformSetAllocInfo, m_uniformSets) == VK_SUCCESS,
				"Uniform sets allocation failed!");

		m_uniformBufferAllocInfos = new VmaAllocationInfo[FRAMES_IN_FLIGHT];
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			size_t size = sizeof(SceneUBO);
			VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

			m_uniformBuffers[i] = _bufferCreate(size, usage, &m_uniformBufferAllocInfos[i]);

			VkDescriptorBufferInfo bufferInfo = {
				.buffer = m_uniformBuffers[i].handle,
				.range = size,
			};

			VkWriteDescriptorSet writeInfo = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_uniformSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo = &bufferInfo,
			};

			vkUpdateDescriptorSets(m_context.device(), 1, &writeInfo, 0, nullptr);
		}
	}

	// sampler

	{
		VkSamplerCreateInfo samplerInfo = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_NEAREST,
			.minFilter = VK_FILTER_NEAREST,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.0f,
			.maxLod = 15.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};

		CHECK_VK_RESULT(vkCreateSampler(m_context.device(), &samplerInfo, nullptr, &m_sampler) == VK_SUCCESS,
				"Sampler creation failed!");
	}

	// image

	{
		VkDescriptorSetLayoutBinding samplerBinding = {
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		VkDescriptorSetLayoutBinding imageBinding = {
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		VkDescriptorSetLayoutBinding bindings[] = {
			samplerBinding,
			imageBinding,
		};

		VkDescriptorSetLayoutCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 2,
			.pBindings = bindings,
		};

		CHECK_VK_RESULT(vkCreateDescriptorSetLayout(m_context.device(), &createInfo, nullptr, &m_textureSetLayout) ==
								VK_SUCCESS,
				"Texture set layout creation failed!");

		VkDescriptorSetAllocateInfo uniformSetAllocInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = m_descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &m_textureSetLayout,
		};

		CHECK_VK_RESULT(vkAllocateDescriptorSets(m_context.device(), &uniformSetAllocInfo, &m_textureSet) == VK_SUCCESS,
				"Texture set allocation failed!");

		VkDescriptorImageInfo samplerInfo = {
			.sampler = m_sampler,
		};

		VkWriteDescriptorSet samplerWriteInfo = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_textureSet,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.pImageInfo = &samplerInfo,
		};

		vkUpdateDescriptorSets(m_context.device(), 1, &samplerWriteInfo, 0, nullptr);
	}

	// checkerboard pipeline

	{
		VkPipelineLayoutCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		};

		CHECK_VK_RESULT(vkCreatePipelineLayout(
								m_context.device(), &createInfo, nullptr, &m_checkerboardPipeline.layout) == VK_SUCCESS,
				"Pipeline layout creation failed!");

		CheckerboardShader shader;
		shader.compile(m_context.device());
		m_checkerboardPipeline.handle = pipelineCreate(m_context.device(), shader.vertex(), shader.fragment(),
				m_checkerboardPipeline.layout, m_context.renderPass(), 0);
	}

	// sprite pipeline

	{
		VkPushConstantRange pushConstantRange = {
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.size = sizeof(ObjectConstants),
		};

		VkDescriptorSetLayout setLayouts[] = {
			m_uniformSetLayout,
			m_textureSetLayout,
		};

		VkPipelineLayoutCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 2,
			.pSetLayouts = setLayouts,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &pushConstantRange,
		};

		CHECK_VK_RESULT(vkCreatePipelineLayout(m_context.device(), &createInfo, nullptr, &m_spritePipeline.layout) ==
								VK_SUCCESS,
				"Pipeline layout creation failed!");

		SpriteShader shader;
		shader.compile(m_context.device());
		m_spritePipeline.handle = pipelineCreate(m_context.device(), shader.vertex(), shader.fragment(),
				m_spritePipeline.layout, m_context.renderPass(), 0);
	}

	m_initialized = true;
}

void RD::windowResize(uint32_t width, uint32_t height) {
	if (m_width == width && m_height == height)
		return;

	m_width = width;
	m_height = height;
	m_resized = true;
}

void RD::create(const char *const *extensions, uint32_t extensionCount, bool validation) {
	m_context.create(extensions, extensionCount, validation);
}

void RD::destroy() {
	vkDeviceWaitIdle(m_context.device());

	if (m_initialized) {
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_context.device(), m_presentSemaphores[i], nullptr);
			vkDestroySemaphore(m_context.device(), m_renderSemaphores[i], nullptr);
			vkDestroyFence(m_context.device(), m_renderFences[i], nullptr);
		}

		vmaDestroyAllocator(m_allocator);
		m_initialized = false;
	}

	m_context.destroy();
}
