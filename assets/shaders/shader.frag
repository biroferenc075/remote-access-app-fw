#version 450


layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;


layout(push_constant) uniform Push {
	mat4 modelMatrix;
	vec3 color;
} push;
layout (location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;
void main() {
 outColor = texture(texSampler, fragTexCoord);
}
//	outColor = vec4(1.0, 1.0, 0.0, 1.0);//vec4(fragTexCoord, 0.0, 1.0);
