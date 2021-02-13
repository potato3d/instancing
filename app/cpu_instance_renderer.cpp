#include <app/cpu_instance_renderer.h>
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
	static const unsigned int SCALE_ATTRIB_ID = 5;
	static const unsigned int ROTATION_ATTRIB_ID = 6;
	static const unsigned int TRANSLATION_ATTRIB_ID = 7;

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// public
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	void cpu_instance_renderer::add_box(const box& b, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= b.extents;
		_box_transforms.push_back(t);
	}

	void cpu_instance_renderer::add_cylinder(const cylinder& c, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(c.radius, c.radius, c.height);
		_cylinder_transforms.push_back(t);
	}

	void cpu_instance_renderer::add_dish(const dish& d, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(d.radius, d.radius, d.height);
		_dish_transforms.push_back(t);
	}

	void cpu_instance_renderer::add_sphere(const sphere& s, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(s.radius, s.radius, s.radius);
		_sphere_transforms.push_back(t);
	}

	bool cpu_instance_renderer::initialize(glb::framebuffer& fbuffer, glb::camera& cam)
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
		shader_builder.bind_vertex_attrib("in_scale", SCALE_ATTRIB_ID);
		shader_builder.bind_vertex_attrib("in_rotation", ROTATION_ATTRIB_ID);
		shader_builder.bind_vertex_attrib("in_translation", TRANSLATION_ATTRIB_ID);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		_shader = shader_builder.get_shader_program();
		_shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());

		return true;
	}

	bool cpu_instance_renderer::finalize()
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

	void cpu_instance_renderer::render()
	{
		_shader.bind();
		for(const auto& t : _box_transforms)
		{
			glVertexAttrib3fv(SCALE_ATTRIB_ID, t.scale.data());
			glVertexAttrib3fv(ROTATION_ATTRIB_ID, t.rotation.data());
			glVertexAttrib3fv(TRANSLATION_ATTRIB_ID, t.translation.data());
			_box_vao.draw();
		}
		for(const auto& t : _cylinder_transforms)
		{
			glVertexAttrib3fv(SCALE_ATTRIB_ID, t.scale.data());
			glVertexAttrib3fv(ROTATION_ATTRIB_ID, t.rotation.data());
			glVertexAttrib3fv(TRANSLATION_ATTRIB_ID, t.translation.data());
			_cylinder_vao.draw();
		}
		for(const auto& t : _dish_transforms)
		{
			glVertexAttrib3fv(SCALE_ATTRIB_ID, t.scale.data());
			glVertexAttrib3fv(ROTATION_ATTRIB_ID, t.rotation.data());
			glVertexAttrib3fv(TRANSLATION_ATTRIB_ID, t.translation.data());
			_dish_vao.draw();
		}
		for(const auto& t : _sphere_transforms)
		{
			glVertexAttrib3fv(SCALE_ATTRIB_ID, t.scale.data());
			glVertexAttrib3fv(ROTATION_ATTRIB_ID, t.rotation.data());
			glVertexAttrib3fv(TRANSLATION_ATTRIB_ID, t.translation.data());
			_sphere_vao.draw();
		}
	}
} // namespace app
