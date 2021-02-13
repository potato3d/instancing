#include <app/static_renderer.h>
#include <tess/tessellator.h>
#include <glb/shader_program_builder.h>
#include <glb/camera.h>
#include <glb/framebuffer.h>

namespace app
{
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// global constants
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	static const unsigned int MAX_BUFFER_SIZE_BYTES = 1 * 1024 * 1024;//1;

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// public
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	void static_renderer::begin_upload()
	{
		_vao_builder.begin();
	}

	void static_renderer::add_box(const box& b, const mat4& transform)
	{
		_add_mesh(tess::tessellate_box(b.extents), transform);
	}

	void static_renderer::add_circular_torus(const circular_torus& ct, const mat4& transform)
	{
		_add_mesh(tess::tessellate_circular_torus(ct.in_radius, ct.out_radius, ct.sweep_angle), transform);
	}

	void static_renderer::add_cone(const cone& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cone(c.top_radius, c.bottom_radius, c.height), transform);
	}

	void static_renderer::add_cone_offset(const cone_offset& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cone_offset(c.top_radius, c.bottom_radius, c.height, c.offset), transform);
	}

	void static_renderer::add_cylinder(const cylinder& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cylinder(c.radius, c.height), transform);
	}

	void static_renderer::add_cylinder_offset(const cylinder_offset& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cylinder_offset(c.radius, c.height, c.offset), transform);
	}

	void static_renderer::add_cylinder_slope(const cylinder_slope& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cylinder_slope(c.radius, c.height, c.top_slope_angles, c.bottom_slope_angles), transform);
	}

	void static_renderer::add_dish(const dish& d, const mat4& transform)
	{
		_add_mesh(tess::tessellate_dish(d.radius, d.height), transform);
	}

	void static_renderer::add_mesh(const tess::triangle_mesh& m, const mat4& transform)
	{
		_add_mesh(m, transform);
	}

	void static_renderer::add_pyramid(const pyramid& p, const mat4& transform)
	{
		_add_mesh(tess::tessellate_pyramid(p.top_extents, p.bottom_extents, p.height, p.offset), transform);
	}

	void static_renderer::add_rectangular_torus(const rectangular_torus& rt, const mat4& transform)
	{
		_add_mesh(tess::tessellate_rectangular_torus(rt.in_radius, rt.out_radius, rt.in_height, rt.sweep_angle), transform);
	}

	void static_renderer::add_sphere(const sphere& s, const mat4& transform)
	{
		_add_mesh(tess::tessellate_sphere(s.radius), transform);
	}

	void static_renderer::end_upload()
	{
		if(!_vao_builder.end())
		{
			throw std::exception();
		}
		_vaos = _vao_builder.get_vertex_arrays();

		io::print("geometries:", _total_geometries);
		io::print("triangles:", _total_triangles);
		io::print("memory:", _total_bytes / 1024.0f / 1024.0f, "MB");
	}

	bool static_renderer::initialize(glb::framebuffer& fbuffer, glb::camera& cam)
	{
		glb::vertex_specification spec;
		spec.setup_vertex_buffer(glb::usage_static_draw, MAX_BUFFER_SIZE_BYTES);
		spec.setup_element_buffer(glb::usage_static_draw, MAX_BUFFER_SIZE_BYTES);
		const auto position_attrib_id = spec.add_vertex_attrib({3, glb::type_float, false, sizeof(tess::vertex), 0});
		const auto normal_attrib_id = spec.add_vertex_attrib({3, glb::type_float, false, sizeof(tess::vertex), sizeof(vec3)});

		if(!_vao_builder.initialize(spec, glb::mode_triangles, glb::type_uint))
		{
			return false;
		}

		glb::shader_program_builder shader_builder;
		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/static_mesh.vert"))
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

		return true;
	}

	bool static_renderer::finalize()
	{
		if(!_shader.destroy())
		{
			return false;
		}
		for(auto& vao : _vaos)
		{
			if(!vao.destroy())
			{
				return false;
			}
		}
		return true;
	}

	void static_renderer::render()
	{
		_shader.bind();
		for(const auto& vao : _vaos)
		{
			vao.draw();
		}
	}

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// private
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	void static_renderer::_add_mesh(tess::triangle_mesh mesh, const mat4& transform)
	{
		// apply transform
		auto transform_normal = transform.to_normal_matrix();
		for(auto& v : mesh.vertices)
		{
			v.position = transform.mul(v.position);
			v.normal = transform_normal.mul3x3(v.normal).normalized();
		}

		// save vertices to render
		_vao_builder.add_mesh(mesh.vertices.data(), mesh.vertices.size(), mesh.elements.data(), mesh.elements.size());

		++_total_geometries;
		_total_triangles += mesh.elements.size()/3;
		_total_bytes += mesh.vertices.size() * sizeof(tess::vertex) + mesh.elements.size() * sizeof(tess::element);
	}
} // namespace app
