#version 460

layout (location = 0) out vec2 UVs;

struct VertexData
{
	float x, y;
	float u, v;
};

layout (binding = 0) readonly buffer vertices { VertexData data[]; } in_vertices;

void main()
{
	VertexData vtx = in_vertices.data[gl_VertexIndex];
	UVs = vec2(vtx.u, vtx.v);
	gl_Position = vec4(vtx.x, vtx.y, 0.0, 1.0);
}