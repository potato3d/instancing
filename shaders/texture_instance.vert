#include "common.vert"

in vec3 in_position;
in vec3 in_normal;

uniform samplerBuffer tex_transforms;

void main()
{
	vec3 scale       = texelFetch(tex_transforms, gl_InstanceID*3+0).xyz;
	vec3 rotation    = texelFetch(tex_transforms, gl_InstanceID*3+1).xyz;
	vec3 translation = texelFetch(tex_transforms, gl_InstanceID*3+2).xyz;

	gl_Position = default_transform(in_position, in_normal, scale, rotation, translation);
}
