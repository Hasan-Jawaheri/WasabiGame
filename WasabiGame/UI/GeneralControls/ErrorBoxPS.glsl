R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform UBO {
	float alpha;
	vec2 spriteSize;
} ubo;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outFragColor;
			
void main() {
	float fBorder = 0.0f;
	vec2 fScreenSpaceTex = inUV * ubo.spriteSize;
	if (fScreenSpaceTex.x < 2 || fScreenSpaceTex.x >= ubo.spriteSize.x - 2 || fScreenSpaceTex.y < 2 || fScreenSpaceTex.y >= ubo.spriteSize.y - 2)
		fBorder = 0.8f;
	outFragColor = vec4(fBorder, fBorder, fBorder, ubo.alpha / 2.0f);
}
)"