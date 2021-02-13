#pragma once
#include <rvm/FileReader.h>
#include <tess/mesh_optimizer.h>
#include <tess/polygon_tessellator.h>

namespace app
{
	class base_renderer;

	class rvm_observer : public rvm::FileReader::IObserver
	{
	public:
		rvm_observer(base_renderer* renderer);

		virtual void beginRead(const char* filename) override;
		virtual void endRead() override;
		virtual void validPrimitive(const rvm::Pyramid& pyramid) override;
		virtual void validPrimitive(const rvm::Box& box) override;
		virtual void validPrimitive(const rvm::RectangularTorus& torus) override;
		virtual void validPrimitive(const rvm::CircularTorus& torus) override;
		virtual void validPrimitive(const rvm::Cone& cone) override;
		virtual void validPrimitive(const rvm::SlopedCone& cone) override;
		virtual void validPrimitive(const rvm::Cylinder& cylinder) override;
		virtual void validPrimitive(const rvm::Sphere& sphere) override;
		virtual void validPrimitive(const rvm::Mesh& mesh) override;
		virtual void validPrimitive(const rvm::Dish& dish) override;
		virtual void beginBlock(rvm::CntBegin& block) override;

	private:
		base_renderer* _renderer;
		tess::polygon_tessellator _ptess;
		tess::mesh_optimizer _meshOpt;
		unsigned char _current_color_id = 0;
	};
} // namespace app
