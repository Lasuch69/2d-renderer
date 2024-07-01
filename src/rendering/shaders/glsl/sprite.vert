#version 450

layout(location = 0) out vec2 texCoord;

layout(set = 0, binding = 0) uniform SceneUBO {
	mat4 PROJECTION_MATRIX;
	mat4 VIEW_MATRIX;
};

layout(push_constant) uniform ObjectConstants {
	mat4 MODEL_MATRIX;
};

const vec2 VERTEX[6] = {
	vec2(-0.5, -0.5),
	vec2(-0.5, 0.5),
	vec2(0.5, -0.5),
	vec2(0.5, -0.5),
	vec2(-0.5, 0.5),
	vec2(0.5, 0.5),
};

void main() {
	vec4 position = MODEL_MATRIX * vec4(VERTEX[gl_VertexIndex], 0.0, 1.0);
	position = floor(position + vec4(0.5));

	texCoord = VERTEX[gl_VertexIndex] + vec2(0.5);
	texCoord.y = 1.0 - texCoord.y;
	gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * position;
}
