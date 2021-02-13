#include <app/rvm_observer.h>
#include <app/base_renderer.h>
#include <tess/polygon_tessellator.h>

namespace app
{
	rvm_observer::rvm_observer(base_renderer* renderer)
		: _renderer(renderer)
	{
	}

	void rvm_observer::beginRead(const char* /*filename*/)
	{
	}

	void rvm_observer::endRead()
	{
	}

	void rvm_observer::validPrimitive(const rvm::Pyramid& rpyramid)
	{
		pyramid tpyramid;
		tpyramid.top_extents = vec2(rpyramid.topLengths);
		tpyramid.bottom_extents = vec2(rpyramid.bottomLengths);
		tpyramid.offset = vec2(rpyramid.offset);
		tpyramid.height = rpyramid.height;
		_renderer->add_pyramid(tpyramid, mat4(rpyramid.transform));
	}

	void rvm_observer::validPrimitive(const rvm::Box& rbox)
	{
		box tbox;
		tbox.extents = vec3(rbox.lengths);
		_renderer->add_box(tbox, mat4(rbox.transform));
	}

	void rvm_observer::validPrimitive(const rvm::RectangularTorus& torus)
	{
		rectangular_torus rtorus;
		rtorus.in_radius = torus.internalRadius;
		rtorus.out_radius = torus.externalRadius;
		rtorus.in_height = torus.height;
		rtorus.sweep_angle = torus.sweepAngle;
		_renderer->add_rectangular_torus(rtorus, mat4(torus.transform));
	}

	void rvm_observer::validPrimitive(const rvm::CircularTorus& torus)
	{
		circular_torus ct;
		ct.in_radius = torus.internalRadius;
		ct.out_radius = torus.externalRadius;
		ct.sweep_angle = torus.sweepAngle;
		_renderer->add_circular_torus(ct, mat4(torus.transform));
	}

	void rvm_observer::validPrimitive(const rvm::Cone& rcone)
	{
		cone tcone;
		tcone.top_radius = rcone.radiusTop;
		tcone.bottom_radius = rcone.radiusBottom;
		tcone.height = rcone.height;
		_renderer->add_cone(tcone, mat4(rcone.transform));
	}

	void rvm_observer::validPrimitive(const rvm::SlopedCone& cone)
	{
		if(cone.bottomSlopeAngle[0] == 0.0f && cone.bottomSlopeAngle[1] == 0.0f &&
		   cone.topSlopeAngle[0] == 0.0f && cone.topSlopeAngle[1] == 0.0f)
		{
			if(cone.radiusBottom == cone.radiusTop)
			{
				cylinder_offset tco;
				tco.radius = cone.radiusBottom;
				tco.height = cone.height;
				tco.offset = vec2(cone.offset);
				_renderer->add_cylinder_offset(tco, mat4(cone.transform));
			}
			else
			{
				cone_offset tco;
				tco.top_radius = cone.radiusTop;
				tco.bottom_radius = cone.radiusBottom;
				tco.height = cone.height;
				tco.offset = vec2(cone.offset);
				_renderer->add_cone_offset(tco, mat4(cone.transform));
			}
		}
		else
		{
			if(cone.radiusBottom == cone.radiusTop)
			{
				cylinder_slope tcs;
				tcs.radius = cone.radiusBottom;
				tcs.height = cone.height;
				tcs.top_slope_angles = vec2(cone.topSlopeAngle);
				tcs.bottom_slope_angles = vec2(cone.bottomSlopeAngle);
				_renderer->add_cylinder_slope(tcs, mat4(cone.transform));
			}
			else
			{
				throw std::exception();
			}
		}

	}

	void rvm_observer::validPrimitive(const rvm::Cylinder& rcylinder)
	{
		cylinder tcylinder;
		tcylinder.radius = rcylinder.radius;
		tcylinder.height = rcylinder.height;
		_renderer->add_cylinder(tcylinder, mat4(rcylinder.transform));
	}

	void rvm_observer::validPrimitive(const rvm::Sphere& rsphere)
	{
		sphere tsphere;
		tsphere.radius = rsphere.radius;
		_renderer->add_sphere(tsphere, mat4(rsphere.transform));
	}

	void rvm_observer::validPrimitive(const rvm::Mesh& mesh)
	{
		// 1- tessellate mesh
		_ptess.begin();

		for(const auto& face : mesh.faces)
		{
			tess::polygon tpoly;
			tpoly.contours.reserve(face.polygons.size());

			for(const auto& poly : face.polygons)
			{
				tess::contour tcontour;
				tcontour.points.reserve(poly.points.size());

				for(const auto& point : poly.points)
				{
					tess::point tpoint;
					tpoint.vertex = vec3(point.vertex);
					tpoint.normal = vec3(point.normal);
					tcontour.points.push_back(tpoint);
				}

				tpoly.contours.push_back(tcontour);
			}

			_ptess.add_polygon(tpoly);
		}

		auto tmesh = _ptess.end();

		// 2- optimize tessellated mesh
		_meshOpt.optimize(tmesh, tess::mesh_optimizer::flag_all_optimizations);

		if(!tmesh.is_valid())
		{
			io::print("invalid mesh!");
			return;
		}

		_renderer->add_mesh(tmesh, mat4(mesh.transform));
	}

	void rvm_observer::validPrimitive(const rvm::Dish& rdish)
	{
		dish tdish;
		tdish.radius = rdish.radius;
		tdish.height = rdish.height;
		_renderer->add_dish(tdish, mat4(rdish.transform));
	}

	void rvm_observer::beginBlock(rvm::CntBegin& block)
	{
		if(block.colorCode < 0 || block.colorCode >= 255)
		{
			throw std::exception();
		}
		_renderer->set_current_color(block.colorCode);
	}
} // namespace app
