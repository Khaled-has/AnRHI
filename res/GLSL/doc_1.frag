#version 460

layout (location = 0) out vec4 out_Color;

layout (location = 0) in vec3 aColor;

layout (binding = 1) uniform sampler2D pTexture;

void main()
{
	out_Color = vec4(aColor, 1.0);
}