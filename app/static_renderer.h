#pragma once
#include <app/base_renderer.h>
#include <glb/shader_program.h>
#include <glb/vertex_array_builder.h>

namespace tess
{
	class triangle_mesh;
} // namespace tess

namespace app
{
	class static_renderer : public app::base_renderer
	{
	public:
		virtual void begin_upload() override;
		virtual void add_box(const box& b, const mat4& transform) override;
		virtual void add_circular_torus(const circular_torus& ct, const mat4& transform) override;
		virtual void add_cone(const cone& c, const mat4& transform) override;
		virtual void add_cone_offset(const cone_offset& c, const mat4& transform) override;
		virtual void add_cylinder(const cylinder& c, const mat4& transform) override;
		virtual void add_cylinder_offset(const cylinder_offset& c, const mat4& transform) override;
		virtual void add_cylinder_slope(const cylinder_slope& c, const mat4& transform) override;
		virtual void add_dish(const dish& d, const mat4& transform) override;
		virtual void add_mesh(const tess::triangle_mesh& m, const mat4& transform) override;
		virtual void add_pyramid(const pyramid& p, const mat4& transform) override;
		virtual void add_rectangular_torus(const rectangular_torus& rt, const mat4& transform) override;
		virtual void add_sphere(const sphere& s, const mat4& transform) override;
		virtual void end_upload() override;

		virtual bool initialize(glb::framebuffer& fbuffer, glb::camera& cam) override;
		virtual bool finalize() override;
		virtual void render() override;

	private:
		void _add_mesh(tess::triangle_mesh mesh, const mat4& transform);

	private:
		glb::shader_program _shader;
		glb::vertex_array_builder _vao_builder;
		vector<glb::vertex_array> _vaos;
		unsigned int _total_geometries = 0;
		unsigned int _total_triangles = 0;
		bl::uint64 _total_bytes = 0;
	};
} // namespace app
