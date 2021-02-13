#include "common.vert"

in vec3 in_position;
in vec3 in_normal;
in int in_tex_offset;
in vec3 in_color;

uniform samplerBuffer tex_transforms;
uniform usamplerBuffer tex_colorIDs;
uniform usamplerBuffer tex_colors;

out vert_color
{
	vec3 diffuse;
} OutColor;

void main()
{
	const mat4 m = mat4(texelFetch(tex_transforms, (in_tex_offset+gl_InstanceID)*3+0),
						texelFetch(tex_transforms, (in_tex_offset+gl_InstanceID)*3+1),
						texelFetch(tex_transforms, (in_tex_offset+gl_InstanceID)*3+2),
						vec4(0.0f, 0.0f, 0.0f, 1.0f));

	gl_Position = default_transform_t(in_position, in_normal, m);

        int color_id = int(texelFetch(tex_colorIDs, in_tex_offset+gl_InstanceID).r);
        OutColor.diffuse = texelFetch(tex_colors, color_id).rgb * vec3(0.00392156862745f);
    //OutColor.diffuse = in_color;
}
