#include <app/parametric_instance_renderer.h>
#include <tess/tessellator.h>
#include <glb/shader_program_builder.h>
#include <glb/camera.h>
#include <glb/framebuffer.h>
#include <glb/opengl.h>

namespace app
{
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// public
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	void parametric_instance_renderer::add_box(const box& b, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= b.extents;
		_box.transforms_buffer.add(t);
	}

	void parametric_instance_renderer::add_circular_torus(const circular_torus& c, const mat4& transform)
	{
		auto t = transformation(transform);
		circular_torus_data ctd;
		ctd.scale = t.scale;
		ctd.rotation = t.rotation;
		ctd.translation = t.translation;
		ctd.radius_in = c.in_radius;
		ctd.radius_out = c.out_radius;
		ctd.sweep_angle = c.sweep_angle;
		_circular_torus.transforms_buffer.add(ctd);
	}

	void parametric_instance_renderer::add_cone(const cone& c, const mat4& transform)
	{
		auto t = transformation(transform);
		cone_data cd;
		cd.scale = t.scale * vec3(1.0f, 1.0f, c.height);
		cd.rotation = t.rotation;
		cd.translation = t.translation;
		cd.bottom_radius = c.bottom_radius;
		cd.top_radius = c.top_radius;
		_cone.transforms_buffer.add(cd);
	}

	void parametric_instance_renderer::add_cone_offset(const cone_offset& c, const mat4& transform)
	{
		auto t = transformation(transform);
		cone_offset_data cod;
		cod.scale = t.scale * vec3(1.0f, 1.0f, c.height);
		cod.rotation = t.rotation;
		cod.translation = t.translation;
		cod.bottom_radius = c.bottom_radius;
		cod.top_radius = c.top_radius;
		cod.offset = c.offset;
		_cone_offset.transforms_buffer.add(cod);
	}

	void parametric_instance_renderer::add_cylinder(const cylinder& c, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(c.radius, c.radius, c.height);
		_cylinder.transforms_buffer.add(t);
	}

	void parametric_instance_renderer::add_cylinder_offset(const cylinder_offset& c, const mat4& transform)
	{
		auto t = transformation(transform);
		cylinder_offset_data cod;
		cod.scale = t.scale * vec3(1.0f, 1.0f, c.height);
		cod.rotation = t.rotation;
		cod.translation = t.translation;
		cod.offset_x = c.offset.x;
		cod.offset_y = c.offset.y;
		cod.radius = c.radius;
		_cylinder_offset.transforms_buffer.add(cod);
	}

	void parametric_instance_renderer::add_cylinder_slope(const cylinder_slope& c, const mat4& transform)
	{
		auto t = transformation(transform);
		cylinder_slope_data csd;
		csd.scale = t.scale;
		csd.rotation = t.rotation;
		csd.translation = t.translation;
		csd.bottom_angles = c.bottom_slope_angles;
		csd.top_angles  = c.top_slope_angles;
		csd.height = c.height;
		csd.radius = c.radius;
		_cylinder_slope.transforms_buffer.add(csd);
	}

	void parametric_instance_renderer::add_dish(const dish& d, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(d.radius, d.radius, d.height);
		_dish.transforms_buffer.add(t);
	}

	void parametric_instance_renderer::add_pyramid(const pyramid& p, const mat4& transform)
	{
		auto t = transformation(transform);
		pyramid_data pd;
		pd.scale = t.scale;
		pd.rotation = t.rotation;
		pd.translation = t.translation;
		pd.offset_x = p.offset.x;
		pd.offset_y = p.offset.y;
		pd.height = p.height;
		pd.bottom_lenghts = p.bottom_extents;
		pd.top_lenghts = p.top_extents;
		_pyramid.transforms_buffer.add(pd);
	}

	void parametric_instance_renderer::add_rectangular_torus(const rectangular_torus& rt, const mat4& transform)
	{
		auto t = transformation(transform);
		rectangular_torus_data rtd;
		rtd.scale = t.scale * vec3(1.0f, 1.0f, rt.in_height);
		rtd.rotation = t.rotation;
		rtd.translation = t.translation;
		rtd.radius_in = rt.in_radius;
		rtd.radius_out = rt.out_radius;
		rtd.sweep_angle = rt.sweep_angle;
		_rectangular_torus.transforms_buffer.add(rtd);
	}

	void parametric_instance_renderer::add_sphere(const sphere& s, const mat4& transform)
	{
		auto t = transformation(transform);
		t.scale *= vec3(s.radius);
		_sphere.transforms_buffer.add(t);
	}

	void parametric_instance_renderer::end_upload()
	{
		unsigned int total = 0;

		total += _box.transforms_buffer.get_size_bytes();
		total += _circular_torus.transforms_buffer.get_size_bytes();
		total += _cone.transforms_buffer.get_size_bytes();
		total += _cone_offset.transforms_buffer.get_size_bytes();
		total += _cylinder.transforms_buffer.get_size_bytes();
		total += _cylinder_offset.transforms_buffer.get_size_bytes();
		total += _cylinder_slope.transforms_buffer.get_size_bytes();
		total += _dish.transforms_buffer.get_size_bytes();
		total += _pyramid.transforms_buffer.get_size_bytes();
		total += _rectangular_torus.transforms_buffer.get_size_bytes();
		total += _sphere.transforms_buffer.get_size_bytes();
		io::print("-- memory parametric:", (double)total / 1024.0 / 1024.0, "MB");

		io::print("total box:", _box.transforms_buffer.get_count());
		io::print("total circular torus:", _circular_torus.transforms_buffer.get_count());
		io::print("total cone:", _cone.transforms_buffer.get_count());
		io::print("total cone offset:", _cone_offset.transforms_buffer.get_count());
		io::print("total cylinder:", _cylinder.transforms_buffer.get_count());
		io::print("total cylinder offset:", _cylinder_offset.transforms_buffer.get_count());
		io::print("total cylinder slope:", _cylinder_slope.transforms_buffer.get_count());
		io::print("total dish:", _dish.transforms_buffer.get_count());
		io::print("total pyramid:", _pyramid.transforms_buffer.get_count());
		io::print("total rectangular torus:", _rectangular_torus.transforms_buffer.get_count());
		io::print("total sphere:", _sphere.transforms_buffer.get_count());
	}

	bool parametric_instance_renderer::initialize(glb::framebuffer& fbuffer, glb::camera& cam)
	{
		// create base meshes
		cad::ParametricBuilder paramBuilder;
		paramBuilder.setMinimumResolutionFlat(16);
		paramBuilder.setMinimumResolutionSmooth(16, 8);

		_flat_vao = paramBuilder.createFlatVAO(1, &_draw_flat_with_caps, &_draw_flat_no_caps);

		if(_flat_vao == 0)
		{
			return false;
		}

		_smooth_vao = paramBuilder.createSmoothVAO(1, &_draw_smooth_with_caps, &_draw_smooth_no_caps);

		if(_smooth_vao == 0)
		{
			return false;
		}

		int transform_texture_unit = 0;
		glb::shader_program_builder shader_builder;
		auto rc = make_random(0.0f, 0.7f);

		// box ---------------------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/box.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_box.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(transformation)))
		{
			return false;
		}
		if(!_box.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_box.transforms_texture.set_data_source(glb::internal_format_rgb32f, _box.transforms_buffer);
		_box.shader = shader_builder.get_shader_program();
		_box.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_box.shader.set_uniform("tex_transforms", transform_texture_unit);
		_box.color = {rc(), rc(), rc()};

		// circular torus ----------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/circularTorus.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_circular_torus.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(circular_torus_data)))
		{
			return false;
		}
		if(!_circular_torus.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_circular_torus.transforms_texture.set_data_source(glb::internal_format_rgba32f, _circular_torus.transforms_buffer);
		_circular_torus.shader = shader_builder.get_shader_program();
		_circular_torus.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_circular_torus.shader.set_uniform("tex_transforms", transform_texture_unit);
		_circular_torus.color = {rc(), rc(), rc()};

		// cone --------------------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/cone.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_cone.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(cone_data)))
		{
			return false;
		}
		if(!_cone.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_cone.transforms_texture.set_data_source(glb::internal_format_rgba32f, _cone.transforms_buffer);
		_cone.shader = shader_builder.get_shader_program();
		_cone.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_cone.shader.set_uniform("tex_transforms", transform_texture_unit);
		_cone.color = {rc(), rc(), rc()};

		// cone offset -------------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/coneOffset.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_cone_offset.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(cone_offset_data)))
		{
			return false;
		}
		if(!_cone_offset.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_cone_offset.transforms_texture.set_data_source(glb::internal_format_rgba32f, _cone_offset.transforms_buffer);
		_cone_offset.shader = shader_builder.get_shader_program();
		_cone_offset.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_cone_offset.shader.set_uniform("tex_transforms", transform_texture_unit);
		_cone_offset.color = {rc(), rc(), rc()};

		// cylinder ----------------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/cylinder.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_cylinder.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 2000000 * sizeof(transformation)))
		{
			return false;
		}
		if(!_cylinder.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_cylinder.transforms_texture.set_data_source(glb::internal_format_rgb32f, _cylinder.transforms_buffer);
		_cylinder.shader = shader_builder.get_shader_program();
		_cylinder.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_cylinder.shader.set_uniform("tex_transforms", transform_texture_unit);
		_cylinder.color = {rc(), rc(), rc()};

		// cylinder offset ---------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/cylinderOffset.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_cylinder_offset.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(cylinder_offset_data)))
		{
			return false;
		}
		if(!_cylinder_offset.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_cylinder_offset.transforms_texture.set_data_source(glb::internal_format_rgba32f, _cylinder_offset.transforms_buffer);
		_cylinder_offset.shader = shader_builder.get_shader_program();
		_cylinder_offset.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_cylinder_offset.shader.set_uniform("tex_transforms", transform_texture_unit);
		_cylinder_offset.color = {rc(), rc(), rc()};

		// cylinder slope ---------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/cylinderSlope.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_cylinder_slope.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(cylinder_slope_data)))
		{
			return false;
		}
		if(!_cylinder_slope.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_cylinder_slope.transforms_texture.set_data_source(glb::internal_format_rgba32f, _cylinder_slope.transforms_buffer);
		_cylinder_slope.shader = shader_builder.get_shader_program();
		_cylinder_slope.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_cylinder_slope.shader.set_uniform("tex_transforms", transform_texture_unit);
		_cylinder_slope.color = {rc(), rc(), rc()};

		// dish --------------------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/dish.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_dish.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(transformation)))
		{
			return false;
		}
		if(!_dish.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_dish.transforms_texture.set_data_source(glb::internal_format_rgb32f, _dish.transforms_buffer);
		_dish.shader = shader_builder.get_shader_program();
		_dish.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_dish.shader.set_uniform("tex_transforms", transform_texture_unit);
		_dish.color = {rc(), rc(), rc()};

		// pyramid------- ----------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/pyramid.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_pyramid.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(pyramid_data)))
		{
			return false;
		}
		if(!_pyramid.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_pyramid.transforms_texture.set_data_source(glb::internal_format_rgba32f, _pyramid.transforms_buffer);
		_pyramid.shader = shader_builder.get_shader_program();
		_pyramid.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_pyramid.shader.set_uniform("tex_transforms", transform_texture_unit);
		_pyramid.color = {rc(), rc(), rc()};

		// rectangular torus -------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/rectangularTorus.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_rectangular_torus.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(rectangular_torus_data)))
		{
			return false;
		}
		if(!_rectangular_torus.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_rectangular_torus.transforms_texture.set_data_source(glb::internal_format_rgba32f, _rectangular_torus.transforms_buffer);
		_rectangular_torus.shader = shader_builder.get_shader_program();
		_rectangular_torus.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_rectangular_torus.shader.set_uniform("tex_transforms", transform_texture_unit);
		_rectangular_torus.color = {rc(), rc(), rc()};

		// sphere ------------------------------------------------------------------------------------------------------

		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/environ/sphere.vert"))
		{
			return false;
		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		if(!_sphere.transforms_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, 1000000 * sizeof(transformation)))
		{
			return false;
		}
		if(!_sphere.transforms_texture.create(transform_texture_unit, glb::target_texture_buffer))
		{
			return false;
		}
		_sphere.transforms_texture.set_data_source(glb::internal_format_rgb32f, _sphere.transforms_buffer);
		_sphere.shader = shader_builder.get_shader_program();
		_sphere.shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_sphere.shader.set_uniform("tex_transforms", transform_texture_unit);
		_sphere.color = {rc(), rc(), rc()};

		return true;
	}

	bool parametric_instance_renderer::finalize()
	{
		return true;
	}

	void parametric_instance_renderer::render()
	{
		// flats -------------------------------------------------------------------------------------------------------

		glBindVertexArray(_flat_vao);

		_box.shader.bind();
		_box.transforms_texture.bind();
		glVertexAttrib3fv(7, _box.color.data());
		glDrawElementsInstanced(_draw_flat_with_caps.mode, _draw_flat_with_caps.count, _draw_flat_with_caps.type, GLB_BYTE_OFFSET(_draw_flat_with_caps.elemOffsetBytes), _box.transforms_buffer.get_count());

		_pyramid.shader.bind();
		_pyramid.transforms_texture.bind();
		glVertexAttrib3fv(7, _pyramid.color.data());
		glDrawElementsInstanced(_draw_flat_with_caps.mode, _draw_flat_with_caps.count, _draw_flat_with_caps.type, GLB_BYTE_OFFSET(_draw_flat_with_caps.elemOffsetBytes), _pyramid.transforms_buffer.get_count());

		_rectangular_torus.shader.bind();
		_rectangular_torus.transforms_texture.bind();
		glVertexAttrib3fv(7, _rectangular_torus.color.data());
		glDrawElementsInstanced(_draw_flat_no_caps.mode, _draw_flat_no_caps.count, _draw_flat_no_caps.type, GLB_BYTE_OFFSET(_draw_flat_no_caps.elemOffsetBytes), _rectangular_torus.transforms_buffer.get_count());

		// smooths -----------------------------------------------------------------------------------------------------

		glBindVertexArray(_smooth_vao);

		_circular_torus.shader.bind();
		_circular_torus.transforms_texture.bind();
		glVertexAttrib3fv(7, _circular_torus.color.data());
		glDrawArraysInstanced(_draw_smooth_no_caps.mode, _draw_smooth_no_caps.first, _draw_smooth_no_caps.count, _circular_torus.transforms_buffer.get_count());

		_cone.shader.bind();
		_cone.transforms_texture.bind();
		glVertexAttrib3fv(7, _cone.color.data());
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cone.transforms_buffer.get_count());

		_cone_offset.shader.bind();
		_cone_offset.transforms_texture.bind();
		glVertexAttrib3fv(7, _cone_offset.color.data());
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cone_offset.transforms_buffer.get_count());

		_cylinder.shader.bind();
		_cylinder.transforms_texture.bind();
		glVertexAttrib3fv(7, _cylinder.color.data());
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cylinder.transforms_buffer.get_count());

		_cylinder_offset.shader.bind();
		_cylinder_offset.transforms_texture.bind();
		glVertexAttrib3fv(7, _cylinder_offset.color.data());
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cylinder_offset.transforms_buffer.get_count());

		_cylinder_slope.shader.bind();
		_cylinder_slope.transforms_texture.bind();
		glVertexAttrib3fv(7, _cylinder_slope.color.data());
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cylinder_slope.transforms_buffer.get_count());

		_dish.shader.bind();
		_dish.transforms_texture.bind();
		glVertexAttrib3fv(7, _dish.color.data());
		glDrawArraysInstanced(_draw_smooth_no_caps.mode, _draw_smooth_no_caps.first, _draw_smooth_no_caps.count, _dish.transforms_buffer.get_count());

		_sphere.shader.bind();
		_sphere.transforms_texture.bind();
		glVertexAttrib3fv(7, _sphere.color.data());
		glDrawArraysInstanced(_draw_smooth_no_caps.mode, _draw_smooth_no_caps.first, _draw_smooth_no_caps.count, _sphere.transforms_buffer.get_count());
	}

	void parametric_instance_renderer::render_color(const vec3& color)
	{
		glVertexAttrib3fv(7, color.data());

		// flats -------------------------------------------------------------------------------------------------------

		glBindVertexArray(_flat_vao);

		_box.shader.bind();
		_box.transforms_texture.bind();
		glDrawElementsInstanced(_draw_flat_with_caps.mode, _draw_flat_with_caps.count, _draw_flat_with_caps.type, GLB_BYTE_OFFSET(_draw_flat_with_caps.elemOffsetBytes), _box.transforms_buffer.get_count());

		_pyramid.shader.bind();
		_pyramid.transforms_texture.bind();
		glDrawElementsInstanced(_draw_flat_with_caps.mode, _draw_flat_with_caps.count, _draw_flat_with_caps.type, GLB_BYTE_OFFSET(_draw_flat_with_caps.elemOffsetBytes), _pyramid.transforms_buffer.get_count());

		_rectangular_torus.shader.bind();
		_rectangular_torus.transforms_texture.bind();
		glDrawElementsInstanced(_draw_flat_no_caps.mode, _draw_flat_no_caps.count, _draw_flat_no_caps.type, GLB_BYTE_OFFSET(_draw_flat_no_caps.elemOffsetBytes), _rectangular_torus.transforms_buffer.get_count());

		// smooths -----------------------------------------------------------------------------------------------------

		glBindVertexArray(_smooth_vao);

		_circular_torus.shader.bind();
		_circular_torus.transforms_texture.bind();
		glDrawArraysInstanced(_draw_smooth_no_caps.mode, _draw_smooth_no_caps.first, _draw_smooth_no_caps.count, _circular_torus.transforms_buffer.get_count());

		_cone.shader.bind();
		_cone.transforms_texture.bind();
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cone.transforms_buffer.get_count());

		_cone_offset.shader.bind();
		_cone_offset.transforms_texture.bind();
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cone_offset.transforms_buffer.get_count());

		_cylinder.shader.bind();
		_cylinder.transforms_texture.bind();
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cylinder.transforms_buffer.get_count());

		_cylinder_offset.shader.bind();
		_cylinder_offset.transforms_texture.bind();
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cylinder_offset.transforms_buffer.get_count());

		_cylinder_slope.shader.bind();
		_cylinder_slope.transforms_texture.bind();
		glDrawArraysInstanced(_draw_smooth_with_caps.mode, _draw_smooth_with_caps.first, _draw_smooth_with_caps.count, _cylinder_slope.transforms_buffer.get_count());

		_dish.shader.bind();
		_dish.transforms_texture.bind();
		glDrawArraysInstanced(_draw_smooth_no_caps.mode, _draw_smooth_no_caps.first, _draw_smooth_no_caps.count, _dish.transforms_buffer.get_count());

		_sphere.shader.bind();
		_sphere.transforms_texture.bind();
		glDrawArraysInstanced(_draw_smooth_no_caps.mode, _draw_smooth_no_caps.first, _draw_smooth_no_caps.count, _sphere.transforms_buffer.get_count());
	}
} // namespace app
