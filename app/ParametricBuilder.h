//---------------------------------------------//
// Developed by:		CAE/Tecgraf/PUC-Rio
// (C) COPYRIGHT 2013:	Petrobras
//---------------------------------------------//
#pragma once

namespace cad
{
	class DrawArraysData
	{
	public:
		DrawArraysData();
		unsigned int mode;
		int first;
		int count;
	};

	class DrawElementsData
	{
	public:
		DrawElementsData();
		unsigned int mode;
		int count;
		unsigned int type;
		unsigned int elemOffsetBytes;
	};

	// assumes DrawData pointers have at least numLOD elements
	// meshes are built in decreasing LOD order: each successive LOD is half the resolution of the previous one
	class ParametricBuilder
	{
	public:
		ParametricBuilder();

		void setMinimumResolutionSmooth(unsigned int resolutionX, unsigned int resolutionY);

		void setMinimumResolutionFlat(unsigned int resolution);

		unsigned int createFlatVAO(unsigned int numLODs, DrawElementsData* withCaps, DrawElementsData* noCaps);

		unsigned int createSmoothVAO(unsigned int numLODs, DrawArraysData* withCaps, DrawArraysData* noCaps);

	private:
		unsigned int _minResolutionX;
		unsigned int _minResolutionY;
		unsigned int _minResolutionFlat;
	};
} // namespace cad
