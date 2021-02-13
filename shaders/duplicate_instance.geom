#include "common.vert"

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VertexData {
    vec3 normal;
    int tex_offset;
    vec3 color;
    int instance_id;
} VertexIn[];

uniform samplerBuffer tex_transforms;
uniform usamplerBuffer tex_colorIDs;
uniform usamplerBuffer tex_colors;

out vert_color
{
	vec3 diffuse;
} OutColor;

void main()
{
	const mat4 m = mat4(texelFetch(tex_transforms, (VertexIn[0].tex_offset+VertexIn[0].instance_id)*3+0),
						texelFetch(tex_transforms, (VertexIn[0].tex_offset+VertexIn[0].instance_id)*3+1),
						texelFetch(tex_transforms, (VertexIn[0].tex_offset+VertexIn[0].instance_id)*3+2),
						vec4(0.0f, 0.0f, 0.0f, 1.0f));

//	int color_id = int(texelFetch(tex_colorIDs, in_tex_offset+gl_InstanceID).r);
//	OutColor.diffuse = texelFetch(tex_colors, color_id).rgb * vec3(0.00392156862745f);
  OutColor.diffuse = VertexIn[0].color;
  
  for(int i = 0; i < 3; i++)
  {
    gl_Position = default_transform_t(gl_in[i].gl_Position.xyz, VertexIn[i].normal, m);
    EmitVertex();
  }
  EndPrimitive();
}
