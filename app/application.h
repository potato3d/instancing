#pragma once
#include <glv/iapplication.h>
#include <glb/engine.h>
#include <app/static_renderer.h>
#include <app/cpu_instance_renderer.h>
#include <app/attrib_instance_renderer.h>
#include <app/texture_instance_renderer.h>
#include <app/duplicate_instance_renderer.h>
#include <app/parametric_instance_renderer.h>
#include <app/combined_instance_renderer.h>

namespace app
{
	class application : public glv::iapplication
	{
	public:
		virtual void display(float* view_matrix) override;
		virtual void reshape(int w, int h) override;
		virtual bool key_press(unsigned char key, int x, int y) override;
		virtual bool initialize() override;
		virtual bool finalize() override;

	private:
		static_renderer _static_renderer;
		cpu_instance_renderer _cpu_instance_renderer;
		attrib_instance_renderer _attrib_instance_renderer;
		texture_instance_renderer _texture_instance_renderer;
		duplicate_instance_renderer _duplicate_instance_renderer;
		parametric_instance_renderer _parametric_instance_renderer;
		combined_instance_renderer _combined_instance_renderer;
		glb::engine _engine;
	};
} // namespace app
