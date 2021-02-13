#pragma once
#include <app/duplicate_instance_renderer.h>
#include <app/parametric_instance_renderer.h>

namespace app
{
	class combined_instance_renderer : public app::base_renderer
	{
	public:
		virtual void add_box(const box& b, const mat4& transform) override;
		virtual void add_circular_torus(const circular_torus& c, const mat4& transform) override;
		virtual void add_cone(const cone& c, const mat4& transform) override;
		virtual void add_cone_offset(const cone_offset& c, const mat4& transform) override;
		virtual void add_cylinder(const cylinder& c, const mat4& transform) override;
		virtual void add_cylinder_offset(const cylinder_offset& c, const mat4& transform) override;
		virtual void add_cylinder_slope(const cylinder_slope& c, const mat4& transform) override;
		virtual void add_dish(const dish& d, const mat4& transform) override;
		virtual void add_pyramid(const pyramid& p, const mat4& transform) override;
		virtual void add_rectangular_torus(const rectangular_torus& rt, const mat4& transform) override;
		virtual void add_sphere(const sphere& s, const mat4& transform) override;
		virtual void add_mesh(const tess::triangle_mesh& m, const mat4& transform) override;
		virtual void end_upload() override;

		virtual bool initialize(glb::framebuffer& fbuffer, glb::camera& cam) override;
		virtual bool finalize() override;
		virtual void render() override;

	private:
		duplicate_instance_renderer _mesh_renderer;
		parametric_instance_renderer _parametric_renderer;
	};
}
