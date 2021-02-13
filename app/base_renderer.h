#pragma once
#include <glb/irenderer.h>
#include <app/geometries.h>
#include <tess/triangle_mesh.h>

namespace app
{
	class base_renderer : public glb::irenderer
	{
	public:
		virtual void begin_upload(){}
		virtual void set_current_color(unsigned char color_id){}
		virtual void add_box(const box& b, const mat4& transform){}
		virtual void add_circular_torus(const circular_torus& c, const mat4& transform){}
		virtual void add_cone(const cone& c, const mat4& transform){}
		virtual void add_cone_offset(const cone_offset& c, const mat4& transform){}
		virtual void add_cylinder(const cylinder& c, const mat4& transform){}
		virtual void add_cylinder_offset(const cylinder_offset& c, const mat4& transform){}
		virtual void add_cylinder_slope(const cylinder_slope& c, const mat4& transform){}
		virtual void add_dish(const dish& d, const mat4& transform){}
		virtual void add_mesh(const tess::triangle_mesh& m, const mat4& transform){}
		virtual void add_pyramid(const pyramid& p, const mat4& transform){}
		virtual void add_rectangular_torus(const rectangular_torus& rt, const mat4& transform){}
		virtual void add_sphere(const sphere& s, const mat4& transform){}
		virtual void end_upload(){}
	};
} // namespace app
