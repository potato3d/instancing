#include "common.vert"

in vec3 in_position;
in vec3 in_normal;

in vec3 in_scale;
in vec3 in_rotation;
in vec3 in_translation;

void main()
{
	gl_Position = default_transform(in_position, in_normal, in_scale, in_rotation, in_translation);
}
