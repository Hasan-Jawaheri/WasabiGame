#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform UBO {
	float alpha;
} ubo;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outFragColor;
			
void main() {
	float fTex = inUV.x;
	if (fTex > 0.75f)
		fTex = (1.0f - fTex) * 2.0f;
	else if (fTex > 0.25f)
		fTex = 0.5f;
	else
		fTex *= 2.0f;
	outFragColor = vec4(0.0f, 0.1f, 0.4f, max(fTex, 0) * ubo.alpha * 3.0f);
}
