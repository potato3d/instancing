#pragma once
#include <bl/bl.h>

namespace app
{
	struct transformation
	{
		explicit transformation(const mat4& matrix);
		vec3 scale;
		vec3 rotation;
		vec3 translation;
	};
} // namespace app
