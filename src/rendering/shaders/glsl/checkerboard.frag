#version 450

layout(location = 0) out vec4 fragColor;

const vec3 WHITE = vec3(0.075);
const vec3 BLACK = vec3(0.025);
const uint GRID_SIZE = 64;

void main() {
	uvec2 coords = uvec2(floor(gl_FragCoord.xy / float(GRID_SIZE)));
	float weight = float((coords.x + coords.y) % 2);
	fragColor.rgb = mix(BLACK, WHITE, weight);
	fragColor.a = 1.0;
}
