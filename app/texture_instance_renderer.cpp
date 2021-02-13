#include <app/texture_instance_renderer.h>
#include <tess/tessellator.h>
#include <glb/shader_program_builder.h>
#include <glb/camera.h>
#include <glb/framebuffer.h>
#include <glb/opengl.h>

namespace app
{
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// global constants
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	static const unsigned int MAX_BUFFER_SIZE_BYTES = 1 * 1024 * 1024;

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// public
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	void texture_instance_renderer::add_box(const box& b, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= b.extents;
		_box_transforms_buffer.add(t);
	}

	void texture_instance_renderer::add_cylinder(const cylinder& c, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(c.radius, c.radius, c.height);
		_cylinder_transforms_buffer.add(t);
	}

	void texture_instance_renderer::add_dish(const dish& d, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(d.radius, d.radius, d.height);
		_dish_transforms_buffer.add(t);
	}

	void texture_instance_renderer::add_sphere(const sphere& s, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(s.radius, s.radius, s.radius);
		_sphere_transforms_buffer.add(t);
	}

	bool texture_instance_renderer::initialize(glb::framebuffer& fbuffer, glb::camera& cam)
	{
		glb::vertex_specification spec;
		spec.setup_vertex_buffer(glb::usage_static_draw, MAX_BUFFER_SIZE_BYTES);
		spec.setup_element_buffer(glb::usage_static_draw, MAX_BUFFER_SIZE_BYTES);
		const auto position_attrib_id = spec.add_vertex_attrib({3, glb::type_float, false, sizeof(tess::vertex), 0});
		const auto normal_attrib_id = spec.add_vertex_attrib({3, glb::type_float, false, sizeof(tess::vertex), sizeof(vec3)});

		glb::vertex_array_builder vao_builder;

		if(!vao_builder.initialize(spec, glb::mode_triangles, glb::type_uint))
		{
			return false;
		}

		auto bmesh = tess::tessellate_box(vec3(1.0f, 1.0f, 1.0f));
		vao_builder.begin();
		vao_builder.add_mesh(bmesh.vertices.data(), bmesh.vertices.size(), bmesh.elements.data(), bmesh.elements.size());
		vao_builder.end();
		_box_vao = vao_builder.get_vertex_arrays()[0];

		auto cmesh = tess::tessellate_cylinder(1.0f, 1.0f);
		vao_builder.begin();
		vao_builder.add_mesh(cmesh.vertices.data(), cmesh.vertices.size(), cmesh.elements.data(), cmesh.elements.size());
		vao_builder.end();
		_cylinder_vao = vao_builder.get_vertex_arrays()[0];

		auto dmesh = tess::tessellate_dish(1.0f, 1.0f);
		vao_builder.begin();
		vao_builder.add_mesh(dmesh.vertices.data(), dmesh.vertices.size(), dmesh.elements.data(), dmesh.elements.size());
		vao_builder.end();
		_dish_vao = vao_builder.get_vertex_arrays()[0];

		auto smesh = tess::tessellate_sphere(1.0f);
		vao_builder.begin();
		vao_builder.add_mesh(smesh.vertices.data(), smesh.vertices.size(), smesh.elements.data(), smesh.elements.size());
		vao_builder.end();
		_sphere_vao = vao_builder.get_vertex_arrays()[0];

		int transform_texture_unit = 0;

		if(!_box_transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 500000 * sizeof(transformation)))
		{
			return false;
		}
		if(!_box_transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_box_transforms_texture.set_data_source(glb::internal_format_rgb32f, _box_transforms_buffer);

		if(!_cylinder_transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 500000 * sizeof(transformation)))
		{
			return false;
		}
		if(!_cylinder_transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_cylinder_transforms_texture.set_data_source(glb::internal_format_rgb32f, _cylinder_transforms_buffer);

		if(!_dish_transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 500000 * sizeof(transformation)))
		{
			return false;
		}
		if(!_dish_transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_dish_transforms_texture.set_data_source(glb::internal_format_rgb32f, _dish_transforms_buffer);

		if(!_sphere_transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 500000 * sizeof(transformation)))
		{
			return false;
		}
		if(!_sphere_transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_sphere_transforms_texture.set_data_source(glb::internal_format_rgb32f, _sphere_transforms_buffer);

		glb::shader_program_builder shader_builder;
		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/texture_instance.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", position_attrib_id);
		shader_builder.bind_vertex_attrib("in_normal", normal_attrib_id);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		_shader = shader_builder.get_shader_program();
		_shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_shader.set_uniform("tex_transforms", transform_texture_unit);

		return true;
	}

	bool texture_instance_renderer::finalize()
	{
		if(!_shader.destroy())
		{
			return false;
		}
		if(!_box_vao.destroy())
		{
			return false;
		}
		if(!_cylinder_vao.destroy())
		{
			return false;
		}
		if(!_dish_vao.destroy())
		{
			return false;
		}
		if(!_sphere_vao.destroy())
		{
			return false;
		}
		return true;
	}

	void texture_instance_renderer::render()
	{
		_shader.bind();
		_box_transforms_texture.bind();
		_box_vao.draw_instanced(_box_transforms_buffer.get_count());
		_cylinder_transforms_texture.bind();
		_cylinder_vao.draw_instanced(_cylinder_transforms_buffer.get_count());
		_dish_transforms_texture.bind();
		_dish_vao.draw_instanced(_dish_transforms_buffer.get_count());
		_sphere_transforms_texture.bind();
		_sphere_vao.draw_instanced(_sphere_transforms_buffer.get_count());
	}
} // namespace app
