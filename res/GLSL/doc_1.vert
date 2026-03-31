#version 460

layout (location = 0) out vec3 aColor;

vec3 colors[3] = {
	vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0)
};

struct VertexData
{
	float x, y;
};

layout (binding = 0) readonly buffer vertices { VertexData data[]; } in_vertices;

void main()
{
	VertexData vtx = in_vertices.data[gl_VertexIndex];
	aColor = colors[gl_VertexIndex];
	gl_Position = vec4(vtx.x, vtx.y, 0.0, 1.0);
}