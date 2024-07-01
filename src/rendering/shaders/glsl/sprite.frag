#version 450

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 fragColor;

layout(set = 1, binding = 0) uniform sampler textureSampler;
layout(set = 1, binding = 1) uniform texture2D textureImage;

void main() {
	vec4 color = texture(sampler2D(textureImage, textureSampler), texCoord);
	fragColor = pow(color, vec4(2.2));
}
