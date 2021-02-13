#pragma once
#include <app/base_renderer.h>
#include <app/transformation.h>
#include <glb/shader_program.h>
#include <glb/vertex_array_builder.h>
#include <glb/texture.h>

namespace app
{
	class texture_instance_renderer : public app::base_renderer
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
		glb::buffer _box_transforms_buffer;
		glb::texture _box_transforms_texture;

		glb::vertex_array _cylinder_vao;
		glb::buffer _cylinder_transforms_buffer;
		glb::texture _cylinder_transforms_texture;

		glb::vertex_array _dish_vao;
		glb::buffer _dish_transforms_buffer;
		glb::texture _dish_transforms_texture;

		glb::vertex_array _sphere_vao;
		glb::buffer _sphere_transforms_buffer;
		glb::texture _sphere_transforms_texture;
	};
} // namespace app
