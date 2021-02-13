#pragma once
#include <app/base_renderer.h>
#include <app/transformation.h>
#include <glb/shader_program.h>
#include <glb/vertex_array_builder.h>
#include <glb/texture.h>
#include <app/ParametricBuilder.h>

namespace app
{
	class parametric_instance_renderer : public app::base_renderer
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
		virtual void end_upload() override;

		virtual bool initialize(glb::framebuffer& fbuffer, glb::camera& cam) override;
		virtual bool finalize() override;
		virtual void render() override;

		void render_color(const vec3& color);

	private:
		struct drawable
		{
			glb::shader_program shader;
			glb::buffer transforms_buffer;
			glb::texture transforms_texture;
			vec3 color;
		};

		struct circular_torus_data
		{
			vec3 scale; float radius_in;
			vec3 rotation; float radius_out;
			vec3 translation; float sweep_angle;
		};

		struct cone_data
		{
			vec3 scale; float pad;
			vec3 rotation; float bottom_radius;
			vec3 translation; float top_radius;
		};

		struct cone_offset_data
		{
			vec3 scale; float pad;
			vec3 rotation; float pad2;
			vec3 translation; float pad3;
			float bottom_radius; float top_radius; vec2 offset;
		};

		struct cylinder_offset_data
		{
			vec3 scale; float offset_x;
			vec3 rotation; float offset_y;
			vec3 translation; float radius;
		};

		struct cylinder_slope_data
		{
			vec3 scale; float radius;
			vec3 rotation; float height;
			vec3 translation; float pad;
			vec2 bottom_angles; vec2 top_angles;
		};

		struct pyramid_data
		{
			vec3 scale; float offset_x;
			vec3 rotation; float offset_y;
			vec3 translation; float height;
			vec2 bottom_lenghts; vec2 top_lenghts;
		};

		struct rectangular_torus_data
		{
			vec3 scale; float radius_in;
			vec3 rotation; float radius_out;
			vec3 translation; float sweep_angle;
		};

		unsigned int _flat_vao;
		cad::DrawElementsData _draw_flat_with_caps;
		cad::DrawElementsData _draw_flat_no_caps;

		unsigned int _smooth_vao;
		cad::DrawArraysData _draw_smooth_with_caps;
		cad::DrawArraysData _draw_smooth_no_caps;

		drawable _box;
		drawable _circular_torus;
		drawable _cone;
		drawable _cone_offset;
		drawable _cylinder;
		drawable _cylinder_offset;
		drawable _cylinder_slope;
		drawable _dish;
		drawable _pyramid;
		drawable _rectangular_torus;
		drawable _sphere;
	};
} // namespace app
