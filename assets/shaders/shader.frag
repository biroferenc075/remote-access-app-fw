#version 450


//layout (location = 0) in vec3 fragColor;
layout (location = 0) in vec2 fragTexCoord;


layout(push_constant) uniform Push {
	mat4 modelMatrix;
	vec3 color;
} push;
layout (location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;
void main() {
 float gamma = 2.2;
 outColor = pow(texture(texSampler, fragTexCoord*vec2(1.0,-1.0)), vec4(1.0/gamma));
}
//outColor = vec4(1.0, 1.0, 0.0, 1.0);//vec4(fragTexCoord, 0.0, 1.0);
