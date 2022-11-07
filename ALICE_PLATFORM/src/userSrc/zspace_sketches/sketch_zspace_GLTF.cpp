//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "nlohmann/tiny_gltf.h"

//#include<headers/zApp/include/zTsStatics.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;

double background = 0.35;

////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zUtilsCore core;

zObjMesh oMesh;
zObjGraph oGraph;

zPointArray positions;

float* myarray;

////// --- GUI OBJECTS ----------------------------------------------------


void setup()
{
	////////////////////////////////////////////////////////////////////////// Enable smooth display
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);

	////////////////////////////////////////////////////////////////////////// ZSPACE
	// initialise model
	model = zModel(100000);

	// read mesh
	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/cube.obj", zOBJ);

	zPointArray pts;
	fnMesh.getVertexPositions(pts);


	const unsigned char* pf = reinterpret_cast<const unsigned char*>(&pts[0].x);

	for (int i = 0; i < pts.size(); i++)
	{
		// ith byte is pf[i]
		printf("\n 0x%x", i);
	}

	

	//gltf
	// Create a model with a single mesh and save it as a gltf file
	tinygltf::Model m;
	tinygltf::Scene scene;
	tinygltf::Mesh mesh;
	tinygltf::Primitive primitive;
	tinygltf::Node node;
	tinygltf::Buffer buffer;
	tinygltf::BufferView bufferView1;
	tinygltf::BufferView bufferView2;
	tinygltf::Accessor accessor1;
	tinygltf::Accessor accessor2;
	tinygltf::Asset asset;

	// This is the raw data buffer. 
	buffer.data = {
		// 6 bytes of indices and two bytes of padding
		0x00,0x00,0x01,0x00,0x02,0x00,0x00,0x00,
		// 36 bytes of floating point numbers
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,
		0x00,0x00,0x00,0x00 };

	

	// "The indices of the vertices (ELEMENT_ARRAY_BUFFER) take up 6 bytes in the
	// start of the buffer.
	bufferView1.buffer = 0;
	bufferView1.byteOffset = 0;
	bufferView1.byteLength = 6;
	bufferView1.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

	// The vertices take up 36 bytes (3 vertices * 3 floating points * 4 bytes)
	// at position 8 in the buffer and are of type ARRAY_BUFFER
	bufferView2.buffer = 0;
	bufferView2.byteOffset = 8;
	bufferView2.byteLength = 36;
	bufferView2.target = TINYGLTF_TARGET_ARRAY_BUFFER;

	// Describe the layout of bufferView1, the indices of the vertices
	accessor1.bufferView = 0;
	accessor1.byteOffset = 0;
	accessor1.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
	accessor1.count = 3;
	accessor1.type = TINYGLTF_TYPE_SCALAR;
	accessor1.maxValues.push_back(2);
	accessor1.minValues.push_back(0);

	// Describe the layout of bufferView2, the vertices themself
	accessor2.bufferView = 1;
	accessor2.byteOffset = 0;
	accessor2.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accessor2.count = 3;
	accessor2.type = TINYGLTF_TYPE_VEC3;
	accessor2.maxValues = { 1.0, 1.0, 0.0 };
	accessor2.minValues = { 0.0, 0.0, 0.0 };

	// Build the mesh primitive and add it to the mesh
	primitive.indices = 0;                 // The index of the accessor for the vertex indices
	primitive.attributes["POSITION"] = 1;  // The index of the accessor for positions
	primitive.material = 0;
	primitive.mode = TINYGLTF_MODE_TRIANGLES;
	mesh.primitives.push_back(primitive);

	// Other tie ups
	node.mesh = 0;
	scene.nodes.push_back(0); // Default scene

	// Define the asset. The version is required
	asset.version = "2.0";
	asset.generator = "tinygltf";

	// Now all that remains is to tie back all the loose objects into the
	// our single model.
	m.scenes.push_back(scene);
	m.meshes.push_back(mesh);
	m.nodes.push_back(node);
	m.buffers.push_back(buffer);
	m.bufferViews.push_back(bufferView1);
	m.bufferViews.push_back(bufferView2);
	m.accessors.push_back(accessor1);
	m.accessors.push_back(accessor2);
	m.asset = asset;

	// Create a simple material
	tinygltf::Material mat;
	mat.pbrMetallicRoughness.baseColorFactor = { 1.0f, 0.9f, 0.9f, 1.0f };
	mat.doubleSided = true;
	m.materials.push_back(mat);

	// Save it to a file
	tinygltf::TinyGLTF gltf;
	gltf.WriteGltfSceneToFile(&m, "data/triangle.gltf",
		true, // embedImages
		true, // embedBuffers
		true, // pretty print
		false); // write binary
	
	
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(oMesh);
	

	// set display element booleans
	oMesh.setDisplayElements(true, true, true);
	

	////////////////////////////////////////////////////////////////////////// Sliders

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	B.addButton(&compute, "compute");
	B.buttons[0].attachToVariable(&compute);
	B.addButton(&display, "display");
	B.buttons[1].attachToVariable(&display);

}

void update(int value)
{
	if (compute)
	{
		zFnMesh fnMesh(oMesh);
		fnMesh.smoothMesh(1);
		fnMesh.to("data/outMesh.json", zJSON);;

		compute = !compute;	
	}
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	if (display)
	{
		// zspace model draw
		model.draw();
		
	}

	for(auto &p : positions) model.displayUtils.drawPoint(p);

	


	//////////////////////////////////////////////////////////

	setup2d();

	glColor3f(0, 0, 0);
	drawString("Vectors #:" + to_string(12), Alice::vec(winW - 350, winH - 500, 0));

	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'p') compute = true;;

	if (k == 'w') compute = true;;

}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
