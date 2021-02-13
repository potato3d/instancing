#pragma once
#include <app/base_renderer.h>
#include <app/transformation.h>
#include <glb/shader_program.h>
#include <glb/vertex_array_builder.h>

namespace app
{
	class attrib_instance_renderer : public app::base_renderer
	{
	public:
		virtual void add_box(const box& b, const mat4& transform) override;
		virtual void add_cylinder(const cylinder& c, const mat4& transform) override;
		virtual void add_dish(const dish& d, const mat4& transform) override;
		virtual void add_sphere(const sphere& s, const mat4& transform) override;
		virtual void end_upload() override;

		virtual bool initialize(glb::framebuffer& fbuffer, glb::camera& cam) override;
		virtual bool finalize() override;
		virtual void render() override;

	private:
		glb::vertex_array_builder _box_vao_builder;
		glb::vertex_array_builder _cylinder_vao_builder;
		glb::vertex_array_builder _dish_vao_builder;
		glb::vertex_array_builder _sphere_vao_builder;

		glb::vertex_specification::attrib_buffer_id _transform_buffer_id = glb::vertex_specification::INVALID;

		unsigned int _box_count = 0;
		unsigned int _cylinder_count = 0;
		unsigned int _dish_count = 0;
		unsigned int _sphere_count = 0;

		glb::shader_program _shader;
		glb::vertex_array _box_vao;
		glb::vertex_array _cylinder_vao;
		glb::vertex_array _dish_vao;
		glb::vertex_array _sphere_vao;
	};
} // namespace app
