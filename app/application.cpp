#include <app/application.h>
#include <app/rvm_observer.h>
#include <rvm/StatsCollector.h>
#include <glv/viewer.h>
#include <bl/bl.h>

namespace app
{
	void application::display(float* view_matrix)
	{
		_engine.set_view(view_matrix);
		_engine.render();
	}

	void application::reshape(int w, int h)
	{
		_engine.resize_screen(w, h);
	}

	bool application::key_press(unsigned char /*key*/, int /*x*/, int /*y*/)
	{
		return false;
	}

	bool application::initialize()
	{
		timer t;

#define COMBINED

#ifdef STATIC
		if(!_engine.initialize(&_static_renderer))
#elif defined(CPU)
		if(!_engine.initialize(&_cpu_instance_renderer))
#elif defined(ATTRIB)
		if(!_engine.initialize(&_attrib_instance_renderer))
#elif defined(TBO)
		if(!_engine.initialize(&_texture_instance_renderer))
#elif defined(MATCHING)
		if(!_engine.initialize(&_duplicate_instance_renderer))
#elif defined(PARAMETRIC)
		if(!_engine.initialize(&_parametric_instance_renderer))
#elif defined(COMBINED)
		if(!_engine.initialize(&_combined_instance_renderer))
#endif
		{
			return false;
		}

#ifdef STATIC
		_static_renderer.begin_upload();
#elif defined(CPU)
		_cpu_instance_renderer.begin_upload();
#elif defined(ATTRIB)
		_attrib_instance_renderer.begin_upload();
#elif defined(TBO)
		_texture_instance_renderer.begin_upload();
#elif defined(MATCHING)
		_duplicate_instance_renderer.begin_upload();
#elif defined(PARAMETRIC)
		_parametric_instance_renderer.begin_upload();
#elif defined(COMBINED)
		_combined_instance_renderer.begin_upload();
#endif

		rvm::FileReader reader;
//		reader.setReadAll(false);
//		reader.setReadBox(true);
//		reader.setReadCircularTorus(true);
//		reader.setReadCone(true);
//		reader.setReadSlopedCone(true);
//		reader.setReadCylinder(true);
//		reader.setReadDish(true);
//		reader.setReadPyramid(true);
//		reader.setReadRectangularTorus(true);
//		reader.setReadSphere(true);
//		reader.setReadMesh(true);

//		reader.setReadMesh(false);

		reader.setReadGroup(true);

//		string baseDir = "/home/potato/Models/abast/COMPERJ/PDMS/";
//		string baseDir = "/home/environ/Models/abast/COMPERJ/PDMS/";

//		string baseDir = "D:/models/abast/COMPERJ/PDMS/";

		string baseDir = "C:/Users/psantos/Downloads//";

		vector<string> files;
		vector<string> dirs;

		// small
//		dirs.push_back(baseDir + "U-4470");
//		glv::viewer::set_default_camera_look_at(5306.69, 2537.95, 54.7586,  5305.83, 2538.41, 54.5342,  0.0f, 0.0f, 1.0f);

		// medium
//		dirs.push_back(baseDir + "U-4710");
//		glv::viewer::set_default_camera_look_at(5609.45, 2945.72, 70.6695,  5608.86, 2946.46, 70.359,  0.0f, 0.0f, 1.0f);

		// large
		dirs.push_back(baseDir + "opengl_minicourse");
		glv::viewer::set_default_camera_look_at(4895.23, 2842.66, 84.3734,  4895.87, 2843.34, 84.0221,  0.0f, 0.0f, 1.0f);

		// massive
//		dirs.push_back(baseDir + "U-2200");
//		dirs.push_back(baseDir + "U-2300");
//		dirs.push_back(baseDir + "U-2400");
//		dirs.push_back(baseDir + "U-2500");
//		dirs.push_back(baseDir + "U-2600");
//		glv::viewer::set_default_camera_look_at(4895.23, 2842.66, 84.3734,  4895.87, 2843.34, 84.0221,  0.0f, 0.0f, 1.0f);

		while(!dirs.empty())
		{
			auto d = dirs.back();
			dirs.pop_back();

			auto contents = path::list_dir(d);
			for(const auto& c : contents)
			{
				if(path::is_dir(c))
				{
//					dirs.push_back(c);
				}
				else if(str::ends_with(c, ".rvm") || str::ends_with(c, ".RVM"))
				{
					files.push_back(c);
				}
			}
		}

		for(const auto& f : files)
		{
			rvm::StatsCollector stats;
			reader.readFile(f.data(), &stats);
			stats.getStats().print();

#ifdef STATIC
			rvm_observer obs(&_static_renderer);
#elif defined(CPU)
			rvm_observer obs(&_cpu_instance_renderer);
#elif defined(ATTRIB)
			rvm_observer obs(&_attrib_instance_renderer);
#elif defined(TBO)
			rvm_observer obs(&_texture_instance_renderer);
#elif defined(MATCHING)
			rvm_observer obs(&_duplicate_instance_renderer);
#elif defined(PARAMETRIC)
			rvm_observer obs(&_parametric_instance_renderer);
#elif defined(COMBINED)
			rvm_observer obs(&_combined_instance_renderer);
#endif

			reader.readFile(f.data(), &obs);
		}

#ifdef STATIC
		_static_renderer.end_upload();
#elif defined(CPU)
		_cpu_instance_renderer.end_upload();
#elif defined(ATTRIB)
		_attrib_instance_renderer.end_upload();
#elif defined(TBO)
		_texture_instance_renderer.end_upload();
#elif defined(MATCHING)
		_duplicate_instance_renderer.end_upload();
#elif defined(PARAMETRIC)
		_parametric_instance_renderer.end_upload();
#elif defined(COMBINED)
		_combined_instance_renderer.end_upload();
#endif

		io::print("time:", t.sec());

		return true;
	}

	bool application::finalize()
	{
		return true;
	}
} // namespace app
