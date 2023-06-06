#define _MAIN_
#ifdef _MAIN_

#include "main.h"

//// zSpace Core Headers
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

using namespace zSpace;

////////////////////////////////////////////////////////// 

double background = 0.75;
bool compute = false;
double timestep = 0.1;

zModel model;

zObjMesh objMesh;
zFnMesh fnMesh;

string path = "data/Plane.obj";

vector<zObjParticle> obj_particles;
vector<zFnParticle> fn_particles;

zVector gravity(0, 0, -0.98);

void setup()
{
	model = zModel(100000);
	model.setShowBufPoints(true, true);

	fnMesh = zFnMesh(objMesh);
	fnMesh.from(path, zOBJ);

	model.addObject(objMesh);

	objMesh.setDisplayElements(true, true, true);

	////////////////////

	zItMeshVertex vIter(objMesh);
	for (vIter.begin(); !vIter.end(); vIter++)
	{
		zObjParticle particle;

		particle.particle.s.p = vIter.getRawPosition();

		if (vIter.checkValency(2))
			particle.particle.fixed = true;

		obj_particles.push_back(particle);
	}



	/*
	* for (int i = 0; obj_particles.size(); i++)
	{
		model.addObject(obj_particles[i]);

		obj_particles[i].setDisplayObject(true);
		obj_particles[i].setDisplayElements(false, false);
	}



	*/




	for (int i = 0; i < obj_particles.size(); i++)
		fn_particles.push_back(zFnParticle(obj_particles[i]));

}

void update(int value)
{
	if (compute)
	{
		for (int i = 0; fn_particles.size(); i++)
			fn_particles[i].addForce(gravity);

		for (int i = 0; i < fn_particles.size(); i++)
			fn_particles[i].integrateForces(timestep, zEuler);

		for (int i = 0; i < fn_particles.size(); i++)
			fn_particles[i].updateParticle(true, true, true);

		//////////////////// Compute edge forces and update mesh


		// TO DO


		//////////////////// Update vertex positions based on particle positions

		zItMeshVertex vIter(objMesh);
		for (vIter.begin(); !vIter.end(); vIter++)
		{
			zVector pos = fn_particles[vIter.getId()].getPosition();
			vIter.setPosition(pos);
		}
	}
}


void draw()
{
	backGround(background);
	drawGrid(50);

	model.draw();

}





void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'c') compute = !compute;
}

void mousePress(int b, int state, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}
#endif // _MAIN_