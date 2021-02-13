#include <app/transformation.h>

namespace app
{
	transformation::transformation(const mat4& matrix)
	{
		scale = matrix.get_scale();
		rotation = matrix.get_rotation_euler();
		translation = matrix.get_translation();
	}
} // namespace app
