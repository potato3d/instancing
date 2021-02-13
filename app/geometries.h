#pragma once
#include <bl/bl.h>

namespace app
{
	struct box
	{
		vec3 extents;
	};

	struct circular_torus
	{
		float in_radius;
		float out_radius;
		float sweep_angle;
	};

	struct cone
	{
		float top_radius;
		float bottom_radius;
		float height;
	};

	struct cone_offset
	{
		float top_radius;
		float bottom_radius;
		float height;
		vec2  offset;
	};

	struct cylinder
	{
		float radius;
		float height;
	};

	struct cylinder_offset
	{
		float radius;
		float height;
		vec2  offset;
	};

	struct cylinder_slope
	{
		float radius;
		float height;
		vec2  top_slope_angles;
		vec2  bottom_slope_angles;
	};

	struct dish
	{
		float radius;
		float height;
	};

	struct pyramid
	{
		vec2 top_extents;
		vec2 bottom_extents;
		vec2 offset;
		float height;
	};

	struct rectangular_torus
	{
		float in_height;
		float in_radius;
		float out_radius;
		float sweep_angle;
	};

	struct sphere
	{
		float radius;
	};
} // namespace app
