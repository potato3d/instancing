//---------------------------------------------//
// Developed by:		CAE/Tecgraf/PUC-Rio
// (C) COPYRIGHT 2013:	Petrobras
//---------------------------------------------//
#include <app/ParametricBuilder.h>
#include <glb/opengl.h>
#include <bl/bl.h>

namespace cad
{
	struct vec4 {float x;float y;float z;float w;};

	DrawArraysData::DrawArraysData() : mode(0),
									   first(0),
									   count(0)
	{
	}

	DrawElementsData::DrawElementsData() : mode(0),
										   count(0),
										   type(0),
										   elemOffsetBytes(0)
	{
	}

	// ---------------------------------------------------------------------------------------------------------------------------------------------------------

	static void createFlatBaseMesh(int resolution, vector<vec4>& outputVertices, vector<bl::uint16>& outputElements)
	{
		bl::uint16 vertexOffset = outputVertices.size();

		// Create cap vertices. The v.x and v.y values are already in local coordinate space. The normal sign goes in the z coordinate, and the information
		// about
		// if the  vertex belongs to a bottom or top cap goes into the w coordinate (0.0 for bottom cap and 1.0 for top cap). The shader will create the normal
		// vector based on the v.z and v.w information, (0.0, 0.0, -1.0) for the bottom cap and (0.0, 0.0, 1.0) for the top cap.

		// Bottom cap
		outputVertices.push_back({-0.5f, -0.5f, -1.0f, 0.0f});
		outputVertices.push_back({0.5f, -0.5f, -1.0f, 0.0f});
		outputVertices.push_back({0.5f, 0.5f, -1.0f, 0.0f});
		outputVertices.push_back({-0.5f, 0.5f, -1.0f, 0.0f});

		outputElements.push_back(0 + vertexOffset);
		outputElements.push_back(3 + vertexOffset);
		outputElements.push_back(1 + vertexOffset);

		outputElements.push_back(1 + vertexOffset);
		outputElements.push_back(3 + vertexOffset);
		outputElements.push_back(2 + vertexOffset);

		// Top cap
		outputVertices.push_back({-0.5f, -0.5f, 1.0f, 1.0f});
		outputVertices.push_back({0.5f, -0.5f, 1.0f, 1.0f});
		outputVertices.push_back({0.5f, 0.5f, 1.0f, 1.0f});
		outputVertices.push_back({-0.5f, 0.5f, 1.0f, 1.0f});

		outputElements.push_back(4 + vertexOffset);
		outputElements.push_back(5 + vertexOffset);
		outputElements.push_back(7 + vertexOffset);

		outputElements.push_back(5 + vertexOffset);
		outputElements.push_back(6 + vertexOffset);
		outputElements.push_back(7 + vertexOffset);

		// Create the remaining vertices considering the resolutionZ. ResolutionZ is the number of segments that will be created.
		// The normal and z information are sent differently here than when the caps were built: the shader will consider that the normal n.x and n.y are in v.z
		// and v.w respectively. The shader will transform zDelta to zero and n.x and n.y will be correct. The z information goes into zDelta and the shader
		// knows it will be there because we add 10 to it. ZDelta should vary between 0 and 1 (plus 10).

		for(int i = 0; i <= resolution; ++i)
		{
			// Identify where in the Z axis (local coordinate space) the vertex should be to inform the rect torus shader.
			float zDelta = static_cast<float>(i) / static_cast<float>(resolution);

			// Sum 10 to zDelta so the shader knows from where to get the z information.
			zDelta += 10;

			outputVertices.push_back({-0.5f, -0.5f, -1.0f, zDelta});
			outputVertices.push_back({-0.5f, -0.5f, zDelta, -1.0f});

			outputVertices.push_back({+0.5f, -0.5f, zDelta, -1.0f});
			outputVertices.push_back({+0.5f, -0.5f, +1.0f, zDelta});

			outputVertices.push_back({+0.5f, +0.5f, +1.0f, zDelta});
			outputVertices.push_back({+0.5f, +0.5f, zDelta, +1.0f});

			outputVertices.push_back({-0.5f, +0.5f, zDelta, +1.0f});
			outputVertices.push_back({-0.5f, +0.5f, -1.0f, zDelta});
		}

		for(int i = 0; i < resolution; ++i)
		{
			// local Coord space:
			//                    ______
			//   z               /     /|
			//   |              /_____/ |Right
			//   |___ y         |Front| |
			//  /               |_____|/
			// x

			// Each element index will have 8 added to skip cap vertices.

			// Left face

			// First Triangle
			outputElements.push_back(((i) * 8 + 1) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 2) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 1) + 8 + vertexOffset);

			//Second Triangle
			outputElements.push_back(((i) * 8 + 1) + 8 + vertexOffset);
			outputElements.push_back(((i) * 8 + 2) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 2) + 8 + vertexOffset);

			// Front face

			// First Triangle
			outputElements.push_back(((i) * 8 + 3) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 4) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 3) + 8 + vertexOffset);

			//Second Triangle
			outputElements.push_back(((i) * 8 + 3) + 8 + vertexOffset);
			outputElements.push_back(((i) * 8 + 4) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 4) + 8 + vertexOffset);

			// Right face

			// First Triangle
			outputElements.push_back(((i + 1) * 8 + 5) + 8 + vertexOffset);
			outputElements.push_back(((i) * 8 + 5) + 8 + vertexOffset);
			outputElements.push_back(((i) * 8 + 6) + 8 + vertexOffset);

			//Second Triangle
			outputElements.push_back(((i) * 8 + 6) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 6) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 5) + 8 + vertexOffset);

			// Back face

			// First Triangle
			outputElements.push_back(((i) * 8 + 0) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 0) + 8 + vertexOffset);
			outputElements.push_back(((i + 1) * 8 + 7) + 8 + vertexOffset);

			//Second Triangle
			outputElements.push_back(((i + 1) * 8 + 7) + 8 + vertexOffset);
			outputElements.push_back(((i) * 8 + 7) + 8 + vertexOffset);
			outputElements.push_back(((i) * 8 + 0) + 8 + vertexOffset);
		}
	}

	static void createSmoothBaseMesh(int resolutionX, int resolutionY, vector<vec2>& outputVertices)
	{
		// resolutionX and resolutionY represent the number of segments in X and Y and not the number of vertices.

		unsigned int prevVerticesSize = outputVertices.size();

		unsigned int tmpTotal = (resolutionX + 1) * (resolutionY + 1);

		vector<vec2> tmp;
		tmp.reserve(tmpTotal);

		// Create a grid and store its vertices in a temporary buffer. x goes from 0 to 1 and y from 1 to 0.
		for(int j = resolutionY; j >= 0; j--)
		{
			for(int i = 0; i <= resolutionX; i++)
			{
				tmp.push_back({static_cast<float>(i) / static_cast<float>(resolutionX), static_cast<float>(j) / static_cast<float>(resolutionY)});
			}
		}

		// Create the Bottom Cap vertices using the grid vertices.
		int first = 0;
		int last  = resolutionX - 1;

		for(int i = 0; i < resolutionX; ++i)
		{
			if(i % 2 == 0)
			{
				outputVertices.push_back({tmp[first].x, -0.5});
				first++;
			}
			else
			{
				outputVertices.push_back({tmp[last].x, -0.5});
				last--;
			}
		}

		// Create the Top Cap vertices using the grid vertices.

		// Force degenerated triangles in order to move to the top cap.
		outputVertices.push_back(outputVertices.back());

		// Force degenerated triangle with vertex already in the top cap.
		outputVertices.push_back({outputVertices.back().x, 1.5});

		// Create the cap
		// The order of vertices to draw the top cap is the inverse order of vertices of the bottom cap.
		// When reverse iterating on the bottom cap vertices, to build the top cap, the iterator starts ignoring the last vertex we added, but uses the
		// penultimate vertex we added to force a degenerate triangle and correct the orientation of the cap.
		for(int i = outputVertices.size() - 2; i >= static_cast<int>(prevVerticesSize); --i)
		{
			outputVertices.push_back({outputVertices[i].x, 1.5});
		}

		// Body

		// Force degenerated triangles in order to move to the Body.
		outputVertices.push_back(outputVertices.back());
		outputVertices.push_back(tmp[0]);

		// Adjust drawing orientation of the body.
		outputVertices.push_back(tmp[0]);

		// Create the body using the grid vertices.
		for(int j = 0; j < resolutionY; j++)
		{
			for(int i = 0; i <= resolutionX; i++)
			{
				outputVertices.push_back(tmp[(resolutionX + 1) * (j) + i]);
				outputVertices.push_back(tmp[(resolutionX + 1) * (j + 1) + i]);
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------------------------------

	ParametricBuilder::ParametricBuilder() : _minResolutionX(4),
											 _minResolutionY(2),
											 _minResolutionFlat(4)
	{
	}

	void ParametricBuilder::setMinimumResolutionSmooth(unsigned int resolutionX, unsigned int resolutionY)
	{
		_minResolutionX = resolutionX;
		_minResolutionY = resolutionY;
	}

	void ParametricBuilder::setMinimumResolutionFlat(unsigned int resolution)
	{
		_minResolutionFlat = resolution;
	}

	unsigned int ParametricBuilder::createFlatVAO(unsigned int numLODs, DrawElementsData* withCaps, DrawElementsData* noCaps)
	{
		bl::uint16 vTotal   = 0;
		unsigned int eTotal = 0;

		unsigned int resolution = _minResolutionFlat;

		for(unsigned int i = 0; i < numLODs; ++i)
		{
			vTotal  = static_cast<bl::uint16>(vTotal + 8 + 8 * (resolution + 1));
			eTotal += 12 + 24 * resolution;   // (2 caps with 6 elements each) + (4 faces with 6 elements each) * resZ
			resolution *= 2;
		}

		vector<vec4> vertices;
		vertices.reserve(vTotal);
		vector<uint16> elements;
		elements.reserve(eTotal);

		for(unsigned int i = 0; i < numLODs; ++i)
		{
			resolution /= 2;

			unsigned int prevElementsSize = elements.size();
			unsigned int prevElementsSizeBytes = elements.size() * sizeof(decltype(elements)::value_type);

			createFlatBaseMesh(resolution, vertices, elements);

			withCaps[i].mode  = GL_TRIANGLES;
			withCaps[i].count = 12 + 24;                               // (2 caps with 6 elements each) + (4 faces with 6 elements each) * 1
			withCaps[i].type  = GL_UNSIGNED_SHORT;
			withCaps[i].elemOffsetBytes = prevElementsSizeBytes;

			noCaps[i].mode  = GL_TRIANGLES;
			noCaps[i].count = elements.size() - prevElementsSize - 12; // (All caps and faces) - (elements of the previous LOD) - (2 caps with 6 elements each)
			noCaps[i].type  = GL_UNSIGNED_SHORT;
			noCaps[i].elemOffsetBytes = prevElementsSizeBytes + 12 * 2; // 12*sizeof(uint16)
		}

		unsigned int vao = 0;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		unsigned int buffer = 0;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(decltype(vertices)::value_type), vertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, 0, 0, GLB_BYTE_OFFSET(0));

		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(decltype(elements)::value_type), elements.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return vao;
	}

	unsigned int ParametricBuilder::createSmoothVAO(unsigned int numLODs, DrawArraysData* withCaps, DrawArraysData* noCaps)
	{
		unsigned int resolutionX = _minResolutionX;
		unsigned int resolutionY = _minResolutionY;

		unsigned int totalVertices = 0;

		for(unsigned int i = 0; i < numLODs; ++i)
		{
			// Update caps and body vertices count.
			unsigned int capVertices  = 2 * resolutionX + 6; // top and bottom caps + degenerated vertices
			unsigned int bodyVertices = 2 * (resolutionX + 1) * (resolutionY + 1) - 2 * (resolutionX + 1);
			totalVertices += capVertices + bodyVertices;

			resolutionX *= 2;
			resolutionY *= 2;
		}

		vector<vec2> vertices;
		vertices.reserve(totalVertices);

		for(unsigned int i = 0; i < numLODs; ++i)
		{
			resolutionX /= 2;
			resolutionY /= 2;
			unsigned int capVertices = 2 * resolutionX + 6;

			unsigned int prevVerticesSize = vertices.size();

			createSmoothBaseMesh(resolutionX, resolutionY, vertices);

			withCaps[i].mode  = GL_TRIANGLE_STRIP;
			withCaps[i].first = prevVerticesSize;
			withCaps[i].count = capVertices + 2 * (resolutionX + 1); // Cap vertices + the first line of the base mesh.

			noCaps[i].mode  = GL_TRIANGLE_STRIP;
			noCaps[i].first = prevVerticesSize + capVertices;
			noCaps[i].count = vertices.size() - noCaps[i].first;
		}

		unsigned int vao = 0;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		unsigned int buffer = 0;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(decltype(vertices)::value_type), vertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, GLB_BYTE_OFFSET(0));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return vao;
	}
} // namespace cad
