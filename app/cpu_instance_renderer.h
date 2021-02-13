#pragma once
#include <app/base_renderer.h>
#include <app/transformation.h>
#include <glb/shader_program.h>
#include <glb/vertex_array_builder.h>

namespace app
{
	class cpu_instance_renderer : public app::base_renderer
	{
	public:
		virtual void add_box(const box& b, const mat4& transform) override;
		virtual void add_cylinder(const cylinder& c, const mat4& transform) override;
		virtual void add_dish(const dish& d, const mat4& transform) override;
		virtual void add_sphere(const sphere& s, const mat4& transform) override;

		virtual bool initialize(glb::framebuffer& fbuffer, glb::camera& cam) override;
		virtual bool finalize() override;
		virtual void render() override;

	private:
		glb::shader_program _shader;

		glb::vertex_array _box_vao;
		vector<transformation> _box_transforms;

		glb::vertex_array _cylinder_vao;
		vector<transformation> _cylinder_transforms;

		glb::vertex_array _dish_vao;
		vector<transformation> _dish_transforms;

		glb::vertex_array _sphere_vao;
		vector<transformation> _sphere_transforms;
	};
} // namespace app
