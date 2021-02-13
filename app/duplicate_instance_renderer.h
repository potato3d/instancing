#pragma once
#include <app/base_renderer.h>
#include <app/transformation.h>
#include <glb/shader_program.h>
#include <glb/vertex_array_builder.h>
#include <glb/texture.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <Eigen/Geometry>
#pragma GCC diagnostic pop

namespace app
{
	typedef Eigen::Vector3d EigenVec3;
	typedef Eigen::Vector4d EigenVec4;
	typedef Eigen::Matrix3d EigenMat3;
	typedef Eigen::Matrix4d EigenMat4;
	typedef Eigen::Matrix<double, 3, Eigen::Dynamic> EigenVec3Array;
	typedef Eigen::Matrix<double, 4, Eigen::Dynamic> EigenVec4Array;

	class duplicate_instance_renderer : public app::base_renderer
	{
	public:
		virtual void set_current_color(unsigned char color_id) override;
		virtual void add_box(const box& b, const mat4& transform) override;
		virtual void add_circular_torus(const circular_torus& ct, const mat4& transform) override;
		virtual void add_cone(const cone& c, const mat4& transform) override;
		virtual void add_cone_offset(const cone_offset& c, const mat4& transform) override;
		virtual void add_cylinder(const cylinder& c, const mat4& transform) override;
		virtual void add_cylinder_offset(const cylinder_offset& c, const mat4& transform) override;
		virtual void add_cylinder_slope(const cylinder_slope& c, const mat4& transform) override;
		virtual void add_dish(const dish& d, const mat4& transform) override;
		virtual void add_mesh(const tess::triangle_mesh& m, const mat4& transform) override;
		virtual void add_pyramid(const pyramid& p, const mat4& transform) override;
		virtual void add_rectangular_torus(const rectangular_torus& rt, const mat4& transform) override;
		virtual void add_sphere(const sphere& s, const mat4& transform) override;
		virtual void end_upload() override;

		virtual bool initialize(glb::framebuffer& fbuffer, glb::camera& cam) override;
		virtual bool finalize() override;
		virtual void render() override;

		void render_color(const vec3& color);

	private:
		void _add_mesh(const tess::triangle_mesh& mesh, const mat4& transform, bool remove_duplicate_vertices = false);

	private:
		struct mat34
		{
			mat34(){}

			explicit mat34(const mat4& m)
			{
				int i = 0;
				data[i++] = m.at(0,0);
				data[i++] = m.at(0,1);
				data[i++] = m.at(0,2);
				data[i++] = m.at(0,3);

				data[i++] = m.at(1,0);
				data[i++] = m.at(1,1);
				data[i++] = m.at(1,2);
				data[i++] = m.at(1,3);

				data[i++] = m.at(2,0);
				data[i++] = m.at(2,1);
				data[i++] = m.at(2,2);
				data[i++] = m.at(2,3);
			}

			mat4 as_mat4() const
			{
				return mat4(data[0], data[1], data[2], data[3],
							data[4], data[5], data[6], data[7],
							data[8], data[9], data[10], data[11],
							0,0,0,1);
			}

			float data[12];
		};

		struct color
		{
			unsigned char rgba[4];
		};

		struct instance_set
		{
			int element_count = 0;
			int element_byte_offset = 0;
			int tex_offset = 0;
			int count = 0;
			vec3 color;
		};

		struct point_set
		{
			tess::triangle_mesh mesh;
			vector<mat34> transforms;
			vector<unsigned char> color_ids;
			EigenVec3Array src;
			EigenVec3 src_mean;
			EigenVec3Array src_local;
			EigenMat3 Aqq;
		};

		unsigned char _current_color_id = 0;
		glb::vertex_array_builder _vao_builder;
		hash_multimap<unsigned int, point_set> _unique_meshes;

		unsigned int _total_vbo_size_bytes = 0;
		unsigned int _total_ebo_size_bytes = 0;

		unsigned int _total_geometries = 0;
		unsigned int _total_triangles = 0;
		unsigned int _total_memory = 0;

		glb::shader_program _shader;
		glb::texture _transform_texture;
		glb::texture _color_ids_texture;
		glb::texture _colors_texture;
		glb::vertex_array _main_vao;
		vector<instance_set> _instance_sets;

		// dynamic data
		vector<mat34> _cpu_transform_buffer;
		vector<mat34> _cpu_transform_buffer2;
		glb::buffer _transform_buffer;
		vector<unsigned char> _cpu_color_id_buffer;
		glb::buffer _color_id_buffer;
	};
} // namespace app
