#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

//layout(location = 0) out vec3 fragColor;
layout(location = 0) out vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionViewMatrix;
	vec3 directionToLight;
} ubo;
layout(push_constant) uniform Push {
	mat4 modelMatrix;
	vec3 color;
} push;
void main() {
gl_Position = ubo.projectionViewMatrix*push.modelMatrix * vec4(inPosition, 1.0);
//gl_Position = vec4(inPosition, 1.0);
//fragColor = vec3(inTexCoord, 0.0);
fragTexCoord = inTexCoord;
}