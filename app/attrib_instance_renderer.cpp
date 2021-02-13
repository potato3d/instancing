#include <app/attrib_instance_renderer.h>
#include <tess/tessellator.h>
#include <glb/shader_program_builder.h>
#include <glb/camera.h>
#include <glb/framebuffer.h>

namespace app
{
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// global constants
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	static const unsigned int MAX_BUFFER_SIZE_BYTES = 1 * 1024 * 1024;

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// public
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	void attrib_instance_renderer::add_box(const box& b, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= b.extents;
		_box_vao_builder.add_instance_attribs(_transform_buffer_id, &t, 1);
		++_box_count;
	}

	void attrib_instance_renderer::add_cylinder(const cylinder& c, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(c.radius, c.radius, c.height);
		_cylinder_vao_builder.add_instance_attribs(_transform_buffer_id, &t, 1);
		++_cylinder_count;
	}

	void attrib_instance_renderer::add_dish(const dish& d, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(d.radius, d.radius, d.height);
		_dish_vao_builder.add_instance_attribs(_transform_buffer_id, &t, 1);
		++_dish_count;
	}

	void attrib_instance_renderer::add_sphere(const sphere& s, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(s.radius, s.radius, s.radius);
		_sphere_vao_builder.add_instance_attribs(_transform_buffer_id, &t, 1);
		++_sphere_count;
	}

	void attrib_instance_renderer::end_upload()
	{
		_box_vao_builder.end();
		_box_vao = _box_vao_builder.get_vertex_arrays()[0];

		_cylinder_vao_builder.end();
		_cylinder_vao = _cylinder_vao_builder.get_vertex_arrays()[0];

		_sphere_vao_builder.end();
		_sphere_vao = _sphere_vao_builder.get_vertex_arrays()[0];
	}

	bool attrib_instance_renderer::initialize(glb::framebuffer& fbuffer, glb::camera& cam)
	{
		glb::vertex_specification spec;
		spec.setup_vertex_buffer(glb::usage_static_draw, MAX_BUFFER_SIZE_BYTES);
		spec.setup_element_buffer(glb::usage_static_draw, MAX_BUFFER_SIZE_BYTES);
		const auto position_attrib_id = spec.add_vertex_attrib({3, glb::type_float, false, sizeof(tess::vertex), 0});
		const auto normal_attrib_id = spec.add_vertex_attrib({3, glb::type_float, false, sizeof(tess::vertex), sizeof(vec3)});

		_transform_buffer_id = spec.add_attrib_buffer(glb::usage_static_draw, 100 * MAX_BUFFER_SIZE_BYTES);
		const auto scale_attrib_id = spec.add_instanced_attrib(_transform_buffer_id, {3, glb::type_float, false, sizeof(transformation), 0});
		const auto rotation_attrib_id = spec.add_instanced_attrib(_transform_buffer_id, {3, glb::type_float, false, sizeof(transformation), sizeof(vec3)});
		const auto translation_attrib_id = spec.add_instanced_attrib(_transform_buffer_id, {3, glb::type_float, false, sizeof(transformation), 2*sizeof(vec3)});

		glb::vertex_array_builder vao_builder;

		if(!vao_builder.initialize(spec, glb::mode_triangles, glb::type_uint))
		{
			return false;
		}

		_box_vao_builder = vao_builder;
		_cylinder_vao_builder = vao_builder;
		_dish_vao_builder = vao_builder;
		_sphere_vao_builder = vao_builder;

		auto bmesh = tess::tessellate_box(vec3(1.0f, 1.0f, 1.0f));
		_box_vao_builder.begin();
		_box_vao_builder.add_mesh(bmesh.vertices.data(), bmesh.vertices.size(), bmesh.elements.data(), bmesh.elements.size());

		auto cmesh = tess::tessellate_cylinder(1.0f, 1.0f);
		_cylinder_vao_builder.begin();
		_cylinder_vao_builder.add_mesh(cmesh.vertices.data(), cmesh.vertices.size(), cmesh.elements.data(), cmesh.elements.size());

		auto dmesh = tess::tessellate_dish(1.0f, 1.0f);
		_dish_vao_builder.begin();
		_dish_vao_builder.add_mesh(dmesh.vertices.data(), dmesh.vertices.size(), dmesh.elements.data(), dmesh.elements.size());

		auto smesh = tess::tessellate_sphere(1.0f);
		_sphere_vao_builder.begin();
		_sphere_vao_builder.add_mesh(smesh.vertices.data(), smesh.vertices.size(), smesh.elements.data(), smesh.elements.size());

		glb::shader_program_builder shader_builder;
		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/attrib_instance.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", position_attrib_id);
		shader_builder.bind_vertex_attrib("in_normal", normal_attrib_id);
		shader_builder.bind_vertex_attrib("in_scale", scale_attrib_id);
		shader_builder.bind_vertex_attrib("in_rotation", rotation_attrib_id);
		shader_builder.bind_vertex_attrib("in_translation", translation_attrib_id);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		_shader = shader_builder.get_shader_program();
		_shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());

		return true;
	}

	bool attrib_instance_renderer::finalize()
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

	void attrib_instance_renderer::render()
	{
		_shader.bind();
		_box_vao.draw_instanced(_box_count);
		_cylinder_vao.draw_instanced(_cylinder_count);
		_dish_vao.draw_instanced(_dish_count);
		_sphere_vao.draw_instanced(_sphere_count);
	}
} // namespace app
