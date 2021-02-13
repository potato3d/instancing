#include <app/duplicate_instance_renderer.h>
#include <glb/camera.h>
#include <glb/framebuffer.h>
#include <glb/shader_program_builder.h>
#include <glb/opengl.h>
#include <tess/tessellator.h>
#include <Eigen/Eigenvalues>
#include <rvm/MaterialTable.h>

namespace app
{
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// global definitions
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	struct EigenOBB
	{
		EigenVec3 center;
		EigenVec3 half_extents;
		EigenMat3 basis;
	};

	struct bbox
	{
		vec3 min = vec3(math::limit_posf());
		float pad0 = 1.0f;
		vec3 max = vec3(math::limit_negf());
		float pad1 = 1.0f;

		void expand(const vec3& v)
		{
			min.x = math::min(min.x, v.x);
			min.y = math::min(min.y, v.y);
			min.z = math::min(min.z, v.z);
			max.x = math::max(max.x, v.x);
			max.y = math::max(max.y, v.y);
			max.z = math::max(max.z, v.z);
		}
	};

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// global constants
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	static const unsigned int MAX_BUFFER_SIZE_BYTES = 5 * 1024 * 1024;
	static const double EPSILON = 1e-3;
	static const int TRANSFORM_TEX_UNIT = 0;
	static const int TEX_OFFSET_ATTRIB = 5;
	static const int COLOR_IDS_TEX_UNIT = 1;
	static const int COLORS_TEX_UNIT = 2;

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// helper functions
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	static bool vec3_equal(const vec3& a, const vec3& b)
	{
		const auto err_x = math::abs(a.x - b.x);
		const auto err_y = math::abs(a.y - b.y);
		const auto err_z = math::abs(a.z - b.z);
		return err_x <= EPSILON && err_y <= EPSILON && err_z <= EPSILON;
	}

	static bool eigen3_equal(const EigenVec3& a, const EigenVec3& b)
	{
		const auto diff = (a-b).cwiseAbs();
		return diff.coeff(0) <= EPSILON && diff.coeff(1) <= EPSILON && diff.coeff(2) <= EPSILON;
	}

	static bool eigen3_dist_equal(const EigenVec3& a, const EigenVec3& b)
	{
		return (a-b).norm() <= EPSILON;
	}

	static bool eigen4_equal(const EigenVec4& a, const EigenVec4& b)
	{
		const auto diff = (a-b).cwiseAbs();
		return diff.coeff(0) <= EPSILON && diff.coeff(1) <= EPSILON && diff.coeff(2) <= EPSILON && a.coeff(3) == 1.0 && b.coeff(3) == 1.0;
	}

	static EigenVec3Array to_eigen(const vector<vec3>& input)
	{
		EigenVec3Array output(3, input.size());

		for(unsigned int i = 0; i < input.size(); ++i)
		{
			output.col(i) = EigenVec3(input[i].x, input[i].y, input[i].z);
		}

		return output;
	}

	static vector<vec3> to_bl(const EigenVec3Array& input)
	{
		vector<vec3> output(input.cols());

		for(unsigned int i = 0; i < input.cols(); ++i)
		{
			output[i] = vec3(input.col(i).coeff(0), input.col(i).coeff(1), input.col(i).coeff(2));
		}

		return output;
	}

	static vec3 vec3_to_bl(const EigenVec3& input)
	{
		return vec3(input.coeff(0), input.coeff(1), input.coeff(2));
	}

	static mat4 mat3_to_bl(const EigenMat3& input)
	{
		return mat4(input.coeff(0,0), input.coeff(0,1), input.coeff(0,2), 0.0f,
					input.coeff(1,0), input.coeff(1,1), input.coeff(1,2), 0.0f,
					input.coeff(2,0), input.coeff(2,1), input.coeff(2,2), 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
	}

	static mat4 mat4_to_bl(const EigenMat4& input)
	{
		return mat4(input.coeff(0,0), input.coeff(0,1), input.coeff(0,2), input.coeff(0,3),
					input.coeff(1,0), input.coeff(1,1), input.coeff(1,2), input.coeff(1,3),
					input.coeff(2,0), input.coeff(2,1), input.coeff(2,2), input.coeff(2,3),
					input.coeff(3,0), input.coeff(3,1), input.coeff(3,2), input.coeff(3,3));
	}

	static EigenMat4 to_mat4(const EigenMat3& input)
	{
		EigenMat4 m;
		m <<	input.coeff(0,0), input.coeff(0,1), input.coeff(0,2), 0.0f,
				input.coeff(1,0), input.coeff(1,1), input.coeff(1,2), 0.0f,
				input.coeff(2,0), input.coeff(2,1), input.coeff(2,2), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f;
		return m;
	}

	static bool is_identity(const mat4& i)
	{
		static const float EPS = 1e-6f;
		return  abs(i.at(0,0) - 1.0) <= EPS && abs(i.at(0,1) - 0.0) <= EPS && abs(i.at(0,2) - 0.0) <= EPS && abs(i.at(0,3) - 0.0) <= EPS &&
				abs(i.at(1,0) - 0.0) <= EPS && abs(i.at(1,1) - 1.0) <= EPS && abs(i.at(1,2) - 0.0) <= EPS && abs(i.at(1,3) - 0.0) <= EPS &&
				abs(i.at(2,0) - 0.0) <= EPS && abs(i.at(2,1) - 0.0) <= EPS && abs(i.at(2,2) - 1.0) <= EPS && abs(i.at(2,3) - 0.0) <= EPS &&
				abs(i.at(3,0) - 0.0) <= EPS && abs(i.at(3,1) - 0.0) <= EPS && abs(i.at(3,2) - 0.0) <= EPS && abs(i.at(3,3) - 1.0) <= EPS;
	}

	static bool is_identity(const EigenMat3& i)
	{
		static const float EPS = 1e-12;
		return  abs(i.coeff(0,0) - 1.0) <= EPS && abs(i.coeff(0,1) - 0.0) <= EPS && abs(i.coeff(0,2) - 0.0) <= EPS &&
				abs(i.coeff(1,0) - 0.0) <= EPS && abs(i.coeff(1,1) - 1.0) <= EPS && abs(i.coeff(1,2) - 0.0) <= EPS &&
				abs(i.coeff(2,0) - 0.0) <= EPS && abs(i.coeff(2,1) - 0.0) <= EPS && abs(i.coeff(2,2) - 1.0) <= EPS;
	}

	static bool is_identity(const EigenMat4& i)
	{
		static const float EPS = 1e-12;
		return  abs(i.coeff(0,0) - 1.0) <= EPS && abs(i.coeff(0,1) - 0.0) <= EPS && abs(i.coeff(0,2) - 0.0) <= EPS && abs(i.coeff(0,3) - 0.0) <= EPS &&
				abs(i.coeff(1,0) - 0.0) <= EPS && abs(i.coeff(1,1) - 1.0) <= EPS && abs(i.coeff(1,2) - 0.0) <= EPS && abs(i.coeff(1,3) - 0.0) <= EPS &&
				abs(i.coeff(2,0) - 0.0) <= EPS && abs(i.coeff(2,1) - 0.0) <= EPS && abs(i.coeff(2,2) - 1.0) <= EPS && abs(i.coeff(2,3) - 0.0) <= EPS &&
				abs(i.coeff(3,0) - 0.0) <= EPS && abs(i.coeff(3,1) - 0.0) <= EPS && abs(i.coeff(3,2) - 0.0) <= EPS && abs(i.coeff(3,3) - 1.0) <= EPS;
	}

	static EigenOBB compute_obb(const EigenVec3Array& src)
	{
		EigenOBB obb;

		// 1. compute geometric center
		EigenVec3 mean = src.rowwise().mean();

		// 2. use PCA to find orthonormal basis centered in average
		EigenVec3Array centered = src.colwise() - mean;
		EigenMat3 covariance_matrix = centered * centered.transpose();

		Eigen::SelfAdjointEigenSolver<EigenMat3> eig(covariance_matrix);
		obb.basis = eig.eigenvectors().rightCols(3);

		// 3. describe src points in local basis: subtract by center and project onto basis vectors
		EigenVec3Array local_src = obb.basis * centered;

		// 4. find minimum and maximum coordinates in local basis
		EigenVec3 local_min = local_src.rowwise().minCoeff();
		EigenVec3 local_max = local_src.rowwise().maxCoeff();

		// 5. compute oriented box center
		EigenVec3 local_center = (local_min + local_max) * 0.5f;
		obb.center = mean + obb.basis * local_center;

		// 6. compute oriented box half-extents (aka half-scale along each local axis)
		obb.half_extents = (local_max - local_min) * 0.5f;

		return obb;
	}

	static mat4 estimate_transform_similarity(const vector<vec3>& src_points, const vector<vec3>& dst_points)
	{
		return mat4_to_bl(Eigen::umeyama(to_eigen(src_points), to_eigen(dst_points), true));
	}

	static mat4 estimate_transform_3x3(const EigenVec3Array& src,
									   const EigenVec3& src_mean, const EigenVec3Array& src_local, const EigenMat3& Aqq,
									   const EigenVec3Array& dst,
									   const EigenVec3& dst_mean, const EigenVec3Array& dst_local,
									   double& error)
	{
		EigenMat3 Apq = dst_local * src_local.transpose();

		EigenMat3 A = Apq * Aqq;
		EigenMat4 A4 = to_mat4(A);

		EigenMat4 M = Eigen::Affine3d(Eigen::Translation3d(dst_mean)).matrix() * A4 * Eigen::Affine3d(Eigen::Translation3d(-src_mean)).matrix();

		error = 0.0;
		for(unsigned int i = 0; i < src.cols(); ++i)
		{
			EigenVec3 s3 = src.col(i);
			EigenVec4 s4(s3.coeff(0), s3.coeff(1), s3.coeff(2), 1.0);
			EigenVec4 ts = M * s4;
			EigenVec3 s(ts.coeff(0), ts.coeff(1), ts.coeff(2));
			EigenVec3 d = dst.col(i);
			error += (d - s).squaredNorm();
		}

		return mat4_to_bl(M);
	}

	static mat4 estimate_transform_4x4(const vector<vec3>& src_pts, const vector<vec3>& dst_pts, double& error)
	{
		EigenVec3Array src = to_eigen(src_pts);
		EigenVec3Array dst = to_eigen(dst_pts);

		EigenVec3 src_mean = src.rowwise().mean();
		EigenVec3Array src_local = src.colwise() - src_mean;

		EigenVec3 dst_mean = dst.rowwise().mean();
		EigenVec3Array dst_local = dst.colwise() - dst_mean;

		EigenVec4Array src_local4(4, src_local.cols());
		src_local4.row(0) = src_local.row(0);
		src_local4.row(1) = src_local.row(1);
		src_local4.row(2) = src_local.row(2);
		src_local4.row(3).setConstant(1.0);

		EigenVec4Array dst_local4(4, dst_local.cols());
		dst_local4.row(0) = dst_local.row(0);
		dst_local4.row(1) = dst_local.row(1);
		dst_local4.row(2) = dst_local.row(2);
		dst_local4.row(3).setConstant(1.0);

		EigenMat4 Apq = dst_local4 * src_local4.transpose();
		EigenMat4 Aqq = (src_local4 * src_local4.transpose()).inverse();

		EigenMat4 A = Apq * Aqq;

		EigenMat4 M = Eigen::Affine3d(Eigen::Translation3d(dst_mean)).matrix() * A * Eigen::Affine3d(Eigen::Translation3d(-src_mean)).matrix();

		error = 0.0;
		for(unsigned int i = 0; i < src.cols(); ++i)
		{
			EigenVec3 s3 = src.col(i);
			EigenVec4 s4(s3.coeff(0), s3.coeff(1), s3.coeff(2), 1.0);
			EigenVec4 ts = M * s4;
			EigenVec3 s(ts.coeff(0), ts.coeff(1), ts.coeff(2));
			EigenVec3 d = dst.col(i);
			error += (d - s).squaredNorm();
		}

		return mat4_to_bl(M);
	}

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// public
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	void duplicate_instance_renderer::set_current_color(unsigned char color_id)
	{
		_current_color_id = color_id;
	}

	void duplicate_instance_renderer::add_box(const box& b, const mat4& transform)
	{
		_add_mesh(tess::tessellate_box(b.extents), transform);
	}

	void duplicate_instance_renderer::add_circular_torus(const circular_torus& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_circular_torus(c.in_radius, c.out_radius, c.sweep_angle), transform);
	}

	void duplicate_instance_renderer::add_cone(const cone& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cone(c.top_radius, c.bottom_radius, c.height), transform);
	}

	void duplicate_instance_renderer::add_cone_offset(const cone_offset& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cone_offset(c.top_radius, c.bottom_radius, c.height, c.offset), transform);
	}

	void duplicate_instance_renderer::add_cylinder(const cylinder& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cylinder(c.radius, c.height), transform);
	}

	void duplicate_instance_renderer::add_cylinder_offset(const cylinder_offset& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cylinder_offset(c.radius, c.height, c.offset), transform);
	}

	void duplicate_instance_renderer::add_cylinder_slope(const cylinder_slope& c, const mat4& transform)
	{
		_add_mesh(tess::tessellate_cylinder_slope(c.radius, c.height, c.top_slope_angles, c.bottom_slope_angles), transform);
	}

	void duplicate_instance_renderer::add_dish(const dish& d, const mat4& transform)
	{
		_add_mesh(tess::tessellate_dish(d.radius, d.height), transform);
	}

	void duplicate_instance_renderer::add_mesh(const tess::triangle_mesh& m, const mat4& transform)
	{
		_add_mesh(m, transform, true);
	}

	void duplicate_instance_renderer::add_pyramid(const pyramid& p, const mat4& transform)
	{
		_add_mesh(tess::tessellate_pyramid(p.top_extents, p.bottom_extents, p.height, p.offset), transform);
	}

	void duplicate_instance_renderer::add_rectangular_torus(const rectangular_torus& rt, const mat4& transform)
	{
		_add_mesh(tess::tessellate_rectangular_torus(rt.in_radius, rt.out_radius, rt.in_height, rt.sweep_angle), transform);
	}

	void duplicate_instance_renderer::add_sphere(const sphere& s, const mat4& transform)
	{
		_add_mesh(tess::tessellate_sphere(s.radius), transform);
	}

	void duplicate_instance_renderer::end_upload()
	{
//		auto aspect = 1.0f;
//		auto fovy = math::to_radians(60.0f);
//		auto yscale = 1.0f / math::tan(fovy*0.5f);
//		auto xscale = yscale / aspect;
//		auto znear = 2.0f;
//		auto zfar = 1000.0f;

//		auto t0 = mat4::translation({0,0,-1});
//		auto t1 = mat4::translation({5,0,0});
//		auto glp = mat4::perspective(fovy, aspect, znear, zfar);
////		glp.at(3,2) = 1.0f;

//		auto dxp = mat4(xscale,   0.0f,                      0.0f,  0.0f,
//						  0.0f, yscale,                      0.0f,  0.0f,
//						  0.0f,   0.0f,         zfar/(znear-zfar), -1.0f,
//						  0.0f,   0.0f, (znear*zfar)/(znear-zfar),  0.0f);

//		add_box({{1,1,1}}, glp.mul(t0));
//		add_pyramid({{1,1}, {2,2}, {0,0}, 1}, t1);




		const auto unique_mesh_count = _unique_meshes.size();
		_instance_sets.reserve(unique_mesh_count);

		glb::vertex_specification spec;
		spec.setup_vertex_buffer(glb::usage_static_draw, _total_vbo_size_bytes);
		spec.setup_element_buffer(glb::usage_static_draw, _total_ebo_size_bytes);
		spec.add_vertex_attrib({3, glb::type_float, false, sizeof(tess::vertex), 0});
		spec.add_vertex_attrib({3, glb::type_float, false, sizeof(tess::vertex), sizeof(vec3)});

		if(!_vao_builder.initialize(spec, glb::mode_triangles, glb::type_uint))
		{
			return;
		}

		_vao_builder.begin();

		_transform_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, _total_geometries * sizeof(mat34));
		_transform_texture.create(TRANSFORM_TEX_UNIT, glb::target_texture_buffer);
		_transform_texture.set_data_source(glb::internal_format_rgba32f, _transform_buffer);

		_color_id_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, _total_geometries * sizeof(unsigned char));
		_color_ids_texture.create(COLOR_IDS_TEX_UNIT, glb::target_texture_buffer);
		_color_ids_texture.set_data_source(glb::internal_format_r8ui, _color_id_buffer);

//		map<int, int> histogram;
//		vector<int> instance_count;

		auto rc = make_random(0.2f, 0.7f);

//#define WRITE(c) {decltype(c) x = c; file.write((char*)&x, sizeof(x));}

//		std::ofstream file("caos.xfm", std::ios::out | std::ios::binary);

//		WRITE(_unique_meshes.size());
//		file.flush();

		for(auto& itr : _unique_meshes)
		{
			auto& ps = itr.second;

//			std::vector<mat34> xfms;

//			for(const auto& t : ps.transforms)
//			{
//				bbox b;
//				for(const auto& v : ps.mesh.vertices)
//				{
//					b.expand(t.as_mat4().mul(v.position));
//				}
//				mat4 m = mat4::translation((b.min+b.max)*0.5f).mul(mat4::scale(b.max-b.min));
//				xfms.push_back(mat34(m));
//			}

//			WRITE(xfms.size());
//			file.flush();
//			file.write((char*)xfms.data(), sizeof(mat34)*xfms.size());
//			file.flush();

			instance_set instances;
			instances.element_count = ps.mesh.elements.size();
			instances.element_byte_offset = _vao_builder._spec.get_element_buffer().get_size_bytes();
			instances.tex_offset = _transform_buffer.get_count();
			instances.count = ps.transforms.size();
			instances.color = vec3(rc(), rc(), rc());
			_instance_sets.push_back(instances);

			_vao_builder.add_mesh(ps.mesh.vertices.data(), ps.mesh.vertices.size(), ps.mesh.elements.data(), ps.mesh.elements.size());

			_transform_buffer.add(ps.transforms.data(), ps.transforms.size());
			_color_id_buffer.add(ps.color_ids.data(), ps.color_ids.size());

			_cpu_transform_buffer.insert(_cpu_transform_buffer.end(), ps.transforms.begin(), ps.transforms.end());
			_cpu_color_id_buffer.insert(_cpu_color_id_buffer.end(), ps.color_ids.begin(), ps.color_ids.end());

			_total_memory += ps.transforms.size() * sizeof(mat34) + ps.color_ids.size() * sizeof(unsigned char);

//			histogram[ps.transforms.size()]++;
//			instance_count.push_back(ps.transforms.size());
		}

//		file.close();

//		_cpu_transform_buffer2.resize(_cpu_transform_buffer.size());

		_vao_builder.end();

		_main_vao = _vao_builder.get_vertex_arrays()[0];

//		int max_count = 0;
//		for(auto c : instance_count)
//		{
//			io::print(c);
//			max_count = math::max(max_count, c);
//		}
//		io::print("max:", max_count);

//		for(auto c : histogram)
//		{
//			io::print("[#instances,#ocurrences]:", c.first, c.second);
//		}

		// free memory
		_unique_meshes = decltype(_unique_meshes)();

		// CAD color table
		rvm::MaterialTable color_table;

		glb::buffer colors_buffer;
		colors_buffer.create(glb::target_texture_buffer, glb::usage_static_draw, color_table.getNumberMaterials() * sizeof(color));

		for(int i = 0; i < color_table.getNumberMaterials(); ++i)
		{
			auto diffuse = vec3(color_table.getMaterial(i).diffuseColor);
			color c;
			c.rgba[0] = 255 * diffuse.x;
			c.rgba[1] = 255 * diffuse.y;
			c.rgba[2] = 255 * diffuse.z;
			if(c.rgba[0] == 0 && c.rgba[1] == 0 && c.rgba[2] == 0)
			{
				c.rgba[0] = 64;
				c.rgba[1] = 64;
				c.rgba[2] = 64;
			}
			colors_buffer.add(c);
		}
		_colors_texture.create(COLORS_TEX_UNIT, glb::target_texture_buffer);
		_colors_texture.set_data_source(glb::internal_format_rgba8ui, colors_buffer);

		io::print("unique meshes:", unique_mesh_count);
		io::print("geometries:", _total_geometries);
		io::print("triangles:", _total_triangles);
		io::print("-- memory matching:", _total_memory / 1024.0f / 1024.0f, "MB");
	}

	bool duplicate_instance_renderer::initialize(glb::framebuffer& fbuffer, glb::camera& cam)
	{
		fbuffer.set_clear_color(0, 1.0f, 1.0f, 1.0f);

		glb::shader_program_builder shader_builder;
		shader_builder.begin();
		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/duplicate_instance.vert"))
		{
			return false;
		}
//		if(!shader_builder.add_file(glb::shader_vertex, "../shaders/pass_through.vert"))
//		{
//			return false;
//		}
//		if(!shader_builder.add_file(glb::shader_geometry, "../shaders/duplicate_instance.geom"))
//		{
//			return false;
//		}
		if(!shader_builder.add_file(glb::shader_fragment, "../shaders/per_pixel_lighting_color.frag"))
		{
			return false;
		}
		shader_builder.bind_vertex_attrib("in_position", 0);
		shader_builder.bind_vertex_attrib("in_normal", 1);
		shader_builder.bind_vertex_attrib("in_tex_offset", TEX_OFFSET_ATTRIB);
		shader_builder.bind_vertex_attrib("in_color", 7);
		shader_builder.bind_draw_buffer("out_color", fbuffer.get_color_buffer_to_display());
		if(!shader_builder.end())
		{
			return false;
		}
		_shader = shader_builder.get_shader_program();
		_shader.bind_uniform_buffer("camera_uniform_block", cam.get_uniform_buffer());
		_shader.set_uniform("tex_transforms", TRANSFORM_TEX_UNIT);
		_shader.set_uniform("tex_colorIDs", COLOR_IDS_TEX_UNIT);
		_shader.set_uniform("tex_colors", COLORS_TEX_UNIT);

		return true;
	}

	bool duplicate_instance_renderer::finalize()
	{
		return true;
	}

	void duplicate_instance_renderer::render()
	{
//		static auto rid = make_random(0, 255);
//		static auto rt = make_random(-1.0f, 1.0f);


//		for(auto& id : _cpu_color_id_buffer)
//		{
//			id = rid();
//		}
//		_color_id_buffer.replace(_cpu_color_id_buffer.data(), _cpu_color_id_buffer.size());

//		for(unsigned int i = 0; i < _cpu_transform_buffer.size(); ++i)
//		{
//			const auto& t = _cpu_transform_buffer[i];
//			_cpu_transform_buffer2[i] = mat34(mat4::translation({rt(), rt(), rt()}).mul(t.as_mat4()));
//		}
//		_transform_buffer.replace(_cpu_transform_buffer2.data(), _cpu_transform_buffer2.size());



		_shader.bind();
		_transform_texture.bind();
		_color_ids_texture.bind();
		_colors_texture.bind();
		_main_vao.bind();
		for(const auto& instances : _instance_sets)
		{
			glVertexAttribI1i(TEX_OFFSET_ATTRIB, instances.tex_offset);
//			glVertexAttrib3fv(7, instances.color.data());
			glDrawElementsInstanced(GL_TRIANGLES, instances.element_count, GL_UNSIGNED_INT, GLB_BYTE_OFFSET(instances.element_byte_offset), instances.count);
		}
	}

	void duplicate_instance_renderer::render_color(const vec3& color)
	{
		_shader.bind();
		_transform_texture.bind();
		_color_ids_texture.bind();
		_colors_texture.bind();
		_main_vao.bind();
		glVertexAttrib3fv(7, color.data());
		for(const auto& instances : _instance_sets)
		{
			glVertexAttribI1i(TEX_OFFSET_ATTRIB, instances.tex_offset);
			glDrawElementsInstanced(GL_TRIANGLES, instances.element_count, GL_UNSIGNED_INT, GLB_BYTE_OFFSET(instances.element_byte_offset), instances.count);
		}
	}

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------
	// private
	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	void duplicate_instance_renderer::_add_mesh(const tess::triangle_mesh& mesh, const mat4& transform, bool remove_duplicate_vertices /*= false*/)
	{
		// 1. apply transform to mesh
		auto new_mesh = mesh;
		const auto ntransform = transform.to_normal_matrix();
		vector<vec3> all_points;
		for(auto& v : new_mesh.vertices)
		{
			v.position = transform.mul(v.position);
			v.normal = ntransform.mul3x3(v.normal);
			all_points.push_back(v.position);
		}

		// 2. get unique positions from transformed mesh
		vector<vec3> new_points;
		if(remove_duplicate_vertices)
		{
			hash_set<vec3> point_hash;
			point_hash.reserve(all_points.size());
			new_points.reserve(all_points.size());
			for(const auto& p : all_points)
			{
				if(point_hash.find(p) == end(point_hash))
				{
					new_points.push_back(p);
					point_hash.emplace(p);
				}
			}
		}
		else
		{
			new_points = all_points;
		}

		EigenVec3Array dst = to_eigen(new_points);
		EigenVec3 dst_mean = dst.rowwise().mean();
		EigenVec3Array dst_local = dst.colwise() - dst_mean;

		// 3. search for candidate mesh with the same number of reference points
		auto range = _unique_meshes.equal_range(new_points.size());

		// 4. for each candidate mesh
		auto best_match = range.second;
		double best_error = EPSILON;
		mat4 best_matrix = mat4::IDENTITY;
		for(auto itr = range.first; itr != range.second; ++itr)
		{
			// 4.1 estimate transformation from candidate mesh to new mesh
			double error = 0.0;
			auto m = estimate_transform_3x3(itr->second.src, itr->second.src_mean, itr->second.src_local, itr->second.Aqq,
											dst, dst_mean, dst_local, error);

			// 4.2 save best match so far
			if(error < best_error)
			{
				best_match = itr;
				best_error = error;
				best_matrix = m;
			}
		}

		if(best_match != range.second)
		{
			best_match->second.transforms.push_back(mat34(best_matrix));
			best_match->second.color_ids.push_back(_current_color_id);
		}
		else
		{
			// 5. if no candidate matched, add new mesh as a new candidate
			_total_vbo_size_bytes += new_mesh.vertices.size() * sizeof(tess::vertex);
			_total_ebo_size_bytes += new_mesh.elements.size() * sizeof(tess::element);

			point_set ps;
			ps.mesh = new_mesh;
			ps.transforms.push_back(mat34(mat4::IDENTITY));
			ps.color_ids.push_back(_current_color_id);

			ps.src = dst;
			ps.src_mean = dst.rowwise().mean();
			ps.src_local = dst.colwise() - ps.src_mean;
			ps.Aqq = (ps.src_local * ps.src_local.transpose()).inverse();

			_unique_meshes.emplace(new_points.size(), ps);

			io::print("unique meshes:", _unique_meshes.size());

			_total_memory += new_mesh.vertices.size() * sizeof(tess::vertex) + new_mesh.elements.size() * sizeof(tess::element);
		}

		++_total_geometries;
		_total_triangles += new_mesh.elements.size()/3;
	}
} // namespace app
