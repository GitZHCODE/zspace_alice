#pragma once

#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif

#define GL_GLEXT_PROTOTYPES



#include <windows.h>		// Header File For Windows
#include <openGL\glew.h>
#include <GL\GL.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <openGL\glext.h>

#include<zSpace\zMesh.h>

#define vertexAttribStride 6
#define vertexColorStride 3
#define edgeIndexStride 1
#define faceIndexStride 1

#define bufferOffset(i) ((void*)(i))
#define GLFloatSize sizeof(GLfloat)
#define GLIntSize sizeof(GLint)

namespace zSpace
{

	class zSPACE_API zBufferObject
	{
	public:

		//----  ATTRIBUTES
		GLuint VBO_vertices;
		GLuint VBO_vertexColors;
		
		GLuint VBO_edgeIndices;
		GLuint VBO_faceIndices;

		GLint max_nVertices;
		
		GLint nVertices;
		GLint nColors;
		GLint nEdges;
		GLint nFaces;

		zBufferObject();

		zBufferObject(GLint _Max_Num_Verts);

		~zBufferObject();

		// APPEND TO BUFFER MEHTODS

		int appendVertexAttributes(vector<zVector>(&_positions), vector<zVector>(&_normals));

		int appendVertexColors(vector<zColor> &_colors);

		int appendEdgeIndices(vector<int> &_edgeIndicies);

		int appendFaceIndices(vector<int> &_faceIndicies);

		void appendMesh(zMesh &inMesh,  bool DihedralEdges = false , double angleThreshold = 45);

		// UPDATE BUFFER MEHTODS

		void updateVertexPositions(vector<zVector>(&_positions), int &startId);

		void updateVertexNormals(vector<zVector>(&_normals), int &startId);

		void updateVertexColors(vector<zColor>(&_colors), int &startId);

		// CLEAR BUFFER MEHTODS

		void clearBufferForRewrite();
	};
}

