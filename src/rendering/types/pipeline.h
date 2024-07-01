#ifndef PIPELINE_H
#define PIPELINE_H

typedef struct VkPipelineLayout_T *VkPipelineLayout;
typedef struct VkPipeline_T *VkPipeline;

typedef struct {
	VkPipelineLayout layout;
	VkPipeline handle;
} Pipeline;

#endif // !PIPELINE_H
