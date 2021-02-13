#include <app/combined_instance_renderer.h>
#include <glb/opengl.h>

namespace app
{
	void combined_instance_renderer::add_box(const box& b, const mat4& transform)
	{
		_parametric_renderer.add_box(b, transform);
	}

	void combined_instance_renderer::add_circular_torus(const circular_torus& c, const mat4& transform)
	{
		_parametric_renderer.add_circular_torus(c, transform);
	}

	void combined_instance_renderer::add_cone(const cone& c, const mat4& transform)
	{
		_parametric_renderer.add_cone(c, transform);
	}

	void combined_instance_renderer::add_cone_offset(const cone_offset& c, const mat4& transform)
	{
		_parametric_renderer.add_cone_offset(c, transform);
	}

	void combined_instance_renderer::add_cylinder(const cylinder& c, const mat4& transform)
	{
		_parametric_renderer.add_cylinder(c, transform);
	}

	void combined_instance_renderer::add_cylinder_offset(const cylinder_offset& c, const mat4& transform)
	{
		_parametric_renderer.add_cylinder_offset(c, transform);
	}

	void combined_instance_renderer::add_cylinder_slope(const cylinder_slope& c, const mat4& transform)
	{
		_parametric_renderer.add_cylinder_slope(c, transform);
	}

	void combined_instance_renderer::add_dish(const dish& d, const mat4& transform)
	{
		_parametric_renderer.add_dish(d, transform);
	}

	void combined_instance_renderer::add_pyramid(const pyramid& p, const mat4& transform)
	{
		_parametric_renderer.add_pyramid(p, transform);
	}

	void combined_instance_renderer::add_rectangular_torus(const rectangular_torus& rt, const mat4& transform)
	{
		_parametric_renderer.add_rectangular_torus(rt, transform);
	}

	void combined_instance_renderer::add_sphere(const sphere& s, const mat4& transform)
	{
		_parametric_renderer.add_sphere(s, transform);
	}

	void combined_instance_renderer::add_mesh(const tess::triangle_mesh& m, const mat4& transform)
	{
		_mesh_renderer.add_mesh(m, transform);
	}

	void combined_instance_renderer::end_upload()
	{
		_mesh_renderer.end_upload();
		_parametric_renderer.end_upload();
	}

	bool combined_instance_renderer::initialize(glb::framebuffer& fbuffer, glb::camera& cam)
	{
		if(!_mesh_renderer.initialize(fbuffer, cam))
		{
			return false;
		}
		if(!_parametric_renderer.initialize(fbuffer, cam))
		{
			return false;
		}
		return true;
	}

	bool combined_instance_renderer::finalize()
	{
		if(!_mesh_renderer.finalize())
		{
			return false;
		}
		if(!_parametric_renderer.finalize())
		{
			return false;
		}
		return true;
	}

#define WIREFRAME 0

	void combined_instance_renderer::render()
	{
#if WIREFRAME
		glDepthFunc(GL_LESS);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

		_mesh_renderer.render();
		_parametric_renderer.render();

#if WIREFRAME
		glDepthFunc(GL_LEQUAL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonOffset(-1, -1);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glLineWidth(2.0f);
		_mesh_renderer.render_color(vec3::ZERO);
		_parametric_renderer.render_color(vec3::ZERO);
		glEnable(GL_POLYGON_OFFSET_LINE);
#endif
	}
}
