//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;
using namespace std;

////////////////////////////////////////////////////////////////////////// TOOL SET

class  zTs2DGS
{
protected:

	/*!	\brief core utilities Object  */
	zUtilsCore coreUtils;


	//--------------------------
	//---- FORM DIAGRAM ATTRIBUTES
	//--------------------------

	/*!	\brief container of  form particle objects  */
	vector<zObjParticle> formParticlesObj;

	/*!	\brief container of form particle function set  */
	vector<zFnParticle> fnFormParticles;

	/*!	\brief container storing the update weights of the form diagram.  */
	vector<float> formVWeights;

	//--------------------------
	//---- FORCE DIAGRAM ATTRIBUTES
	//--------------------------

	/*!	\brief container of  force particle objects  */
	vector<zObjParticle> forceParticlesObj;

	/*!	\brief container of force particle function set  */
	vector<zFnParticle> fnForceParticles;

	/*!	\brief container storing the update weights of the force diagram.  */
	vector<float> forceVWeights;

	//--------------------------
	//---- ATTRIBUTES
	//--------------------------

	/*!	\brief container of force densities  */
	vector<float> forceDensities;

	/*!	\brief container of indicies of fixed vertices  */
	vector<int> fixedVertices;

	/*!	\brief container of booleans of fixed vertices  */
	vector<bool> fixedVerticesBoolean;

	/*!	\brief container storing the corresponding force edge per form edge.  */
	vector<int> forceEdge_formEdge;

	/*!	\brief container storing the corresponding form edge per force edge.  */
	vector<int> formEdge_forceEdge;

	/*!	\brief container storing the form edges in tension.  */
	vector<bool> form_tensionEdges;

	/*!	\brief container storing the force edges in tension.  */
	vector<bool> force_tensionEdges;

	/*!	\brief container storing the form edges in tension.  */
	vector<bool> form_ExternalForceEdges;

	/*!	\brief container storing the force edges in tension.  */
	vector<bool> force_ExternalForceEdges;

	/*!	\brief container storing the horizontal equilibrium target for form edges.  */
	vector<zVector> targetEdges_form;

	/*!	\brief container storing the horizontal equilibrium target for force edges.  */
	vector<zVector> targetEdges_force;

public:

	/*!	\brief form mesh Object  */
	zObjMesh oForm;

	/*!	\brief force mesh Object  */
	zObjMesh oForce;

	
	zTs2DGS() {};

	~zTs2DGS() {};

	//---- CREATE METHODS

	void createfromFile(string path, zFileTpye fileType, zDiagramType diagramType)
	{
		if (diagramType == zDiagramType::zFormDiagram)
		{
			zFnMesh fnForm(oForm);
			fnForm.from(path, fileType);


			// set fixed verttices from color
			zColorArray vColors;
			fnForm.getVertexColors(vColors);
			
			zIntArray fixedVerts;
			for (zItMeshVertex v(oForm); !v.end(); v++)
			{
				if (v.getColor() == zBLACK)fixedVerts.push_back(v.getId());
			}

			setConstraints(diagramType, fixedVerts);

			form_tensionEdges.clear();
			form_tensionEdges.assign(fnForm.numHalfEdges(), false);

			form_ExternalForceEdges.clear();
			form_ExternalForceEdges.assign(fnForm.numHalfEdges(), false);

			setTensionEdges(zFormDiagram);
			setExternalForceEdges(zFormDiagram);
			setElementColorDomain(zFormDiagram);
			setVertexWeights(zFormDiagram);
		}

		if (diagramType == zDiagramType::zForceDiagram)
		{
			zFnMesh fnForce(oForce);
			fnForce.from(path, fileType);

			force_tensionEdges.clear();
			force_tensionEdges.assign(fnForce.numHalfEdges(), false);

			force_ExternalForceEdges.clear();
			force_ExternalForceEdges.assign(fnForce.numHalfEdges(), false);

			setTensionEdges(zForceDiagram);
			setExternalForceEdges(zForceDiagram);
			setElementColorDomain(zForceDiagram);
			setVertexWeights(zForceDiagram);
		}

	}

	void createForceFromForm(bool rotate90)
	{

		zFnMesh fnForm(oForm);

		zPointArray positions;
		zIntArray polyConnects;
		zIntArray polyCounts;


		int n_v = fnForm.numVertices();
		int n_e = fnForm.numHalfEdges();
		int n_f = fnForm.numPolygons();


		zPointArray faceCenters;
		fnForm.getCenters(zFaceData, faceCenters);
		positions = faceCenters;

		vector<int> formHalfEdge_forceVertex;

		for (zItMeshHalfEdge he(oForm); !he.end(); he++)
		{
			formHalfEdge_forceVertex.push_back(-1);

			if (!he.onBoundary())
			{
				formHalfEdge_forceVertex[he.getId()] = he.getFace().getId();
			}
		}

		for (zItMeshHalfEdge he(oForm); !he.end(); he++)
		{
			int i = he.getId();
			if (formHalfEdge_forceVertex[i] == -1)
			{

				// check if both the  vertices of the boundary edge arent fixed

				bool edgeVertsFixed = false;


				vector<int> eVerts;
				he.getVertices(eVerts);

				int v1 = eVerts[0];
				int v2 = eVerts[1];


				if (fixedVerticesBoolean[v1] && fixedVerticesBoolean[v2])
				{
					edgeVertsFixed = true;

				}

				// walk on boundary and find all vertices between two fixed vertices
				if (!edgeVertsFixed)
				{
					vector<int>  boundaryEdges;
					vector<int>  boundaryVertices;

					zItMeshHalfEdge start = he;
					zItMeshHalfEdge e = he;

					bool exit = false;


					// walk prev
					do
					{

						if (fixedVerticesBoolean[e.getSym().getVertex().getId()])
						{

							exit = true;
							boundaryVertices.push_back(e.getSym().getVertex().getId());
						}

						boundaryEdges.push_back(e.getId());
						boundaryVertices.push_back(e.getVertex().getId());

						if (e.getPrev().isActive())
						{
							e = e.getPrev();
						}
						else exit = true;

					} while (e != start && !exit);


					// walk next 
					// checking if the prev walk as completed the full edge loop
					if (e != start)
					{
						bool exit = false;
						e = start;
						do
						{
							if (fixedVerticesBoolean[e.getVertex().getId()])
							{
								exit = true;

							}

							if (exit) continue;

							if (e.getNext().isActive())
							{
								e = e.getNext();
								boundaryVertices.push_back(e.getVertex().getId());
								boundaryEdges.push_back(e.getId());

								if (fixedVerticesBoolean[e.getVertex().getId()])
								{
									exit = true;


								}

							}
							else exit = true;


						} while (e != start && !exit);
					}


					if (boundaryEdges.size() > 1)
					{

						int vertId = positions.size();
						zVector newPos;
						for (int j = 0; j < boundaryEdges.size(); j++)
						{
							formHalfEdge_forceVertex[boundaryEdges[j]] = vertId;
						}



						for (int j = 0; j < boundaryVertices.size(); j++)
						{
							zItMeshVertex v(oForm, boundaryVertices[j]);
							zVector pos = v.getPosition();
							newPos += pos;
						}



						newPos /= boundaryVertices.size();
						positions.push_back(newPos);
					}

				}


			}
		}

		for (zItMeshVertex v(oForm); !v.end(); v++)
		{
			int i = v.getId();

			if (!fixedVerticesBoolean[i])
			{
				vector<int> cEdges;
				v.getConnectedHalfEdges(cEdges);

				if (cEdges.size() > 2)
				{
					for (int j = 0; j < cEdges.size(); j++)
					{
						int vertId = formHalfEdge_forceVertex[cEdges[j]];

						polyConnects.push_back(vertId);
					}

					polyCounts.push_back(cEdges.size());
				}

			}


		}


		zFnMesh fnForce(oForce);
		fnForce.create(positions, polyCounts, polyConnects);

		//printf("\n forceMesh: %i %i %i", out.numVertices(), out.numEdges(), out.numPolygons());

		if (rotate90)
		{
			// bounding box
			zVector minBB, maxBB;
			vector<zVector> vertPositions;
			fnForce.getVertexPositions(vertPositions);

			coreUtils.getBounds(vertPositions, minBB, maxBB);

			zVector cen = (maxBB + minBB) * 0.5;

			zVector rotAxis(0, 0, 1);

			for (int i = 0; i < vertPositions.size(); i++)
			{
				vertPositions[i] -= cen;
				vertPositions[i] = vertPositions[i].rotateAboutAxis(rotAxis, -90);
			}


			fnForce.setVertexPositions(vertPositions);

			//printf("\n working!");
		}


		// compute forceEdge_formEdge
		forceEdge_formEdge.clear();
		for (int i = 0; i < fnForce.numHalfEdges(); i++)
		{
			forceEdge_formEdge.push_back(-1);
		}

		// compute form edge to force edge	
		formEdge_forceEdge.clear();
		
		for (zItMeshHalfEdge e_form(oForm); !e_form.end(); e_form++)
		{
			int i = e_form.getId();

			int v1 = formHalfEdge_forceVertex[i];
			int v2 = (i % 2 == 0) ? formHalfEdge_forceVertex[i + 1] : formHalfEdge_forceVertex[i - 1];

			zItMeshHalfEdge e_force;
			bool chk = fnForce.halfEdgeExists(v1, v2, e_force);

			int eId = -1;

			if (chk)
			{
				zVector e_Form_vec = e_form.getVector();
				e_Form_vec.normalize();

				zVector e_Force_vec = e_force.getVector();
				e_Force_vec.normalize();

				eId = e_force.getId();

				// for tension edge point to the edge in the opposite direction
				//int symId = fnForm.getSymIndex(i);
				//if (fnForm.onBoundary(i, zHalfEdgeData) || fnForm.onBoundary(symId, zHalfEdgeData))
				//{
				//if (form_tensionEdges[i])
				//{
				//	if (e_Form_vec*e_Force_vec > 0) eId = e_force.getSym().getId();
				//}
				////for compression edge point to the edge in the same direction
				//else
				//{
				//	if (e_Form_vec*e_Force_vec < 0) eId = e_force.getSym().getId();
				//}
				//}			



			}

			formEdge_forceEdge.push_back(eId);

			if (formEdge_forceEdge[i] != -1)
			{
				forceEdge_formEdge[formEdge_forceEdge[i]] = i;

			}
		}


		force_tensionEdges.clear();
		force_tensionEdges.assign(fnForce.numHalfEdges(), false);

		force_ExternalForceEdges.clear();
		force_ExternalForceEdges.assign(fnForce.numHalfEdges(), false);


		setVertexWeights(zForceDiagram);

		setForceTensionEdgesfromForm();
		setForceExternalEdgesfromForm();
		setElementColorDomain(zForceDiagram);
	}

	//---- 2DGS METHODS

	bool equilibrium(bool& computeTargets, float formWeight, float dT, zIntergrationType type, int numIterations, float angleTolerance, float minMax_formEdge, float minMax_forceEdge, zDomainFloat dev, bool colorEdges, bool printInfo)
	{
		// compute horizontal equilibrium targets
		if (computeTargets)
		{
			getHorizontalTargets(formWeight);

			if (formVWeights.size() == 0) setVertexWeights(zDiagramType::zFormDiagram);
			if (forceVWeights.size() == 0) setVertexWeights(zDiagramType::zForceDiagram);

			computeTargets = !computeTargets;
		}

		// update diagrams

		if (formWeight != 1.0)
		{
			updateFormDiagram(minMax_formEdge, dT, type, numIterations);
		}

		if (formWeight != 0.0)
		{
			updateForceDiagram(minMax_forceEdge, dT, type, numIterations);
		}

		// check deviations
		dev = zDomainFloat();
		bool out = checkHorizontalParallelity(dev, angleTolerance, colorEdges, printInfo);

		if (out)
		{

			setElementColorDomain(zFormDiagram);
			setElementColorDomain(zForceDiagram);
		}

		return out;
	}

	//--- SET METHODS 

	void setElementColorDomain(zDiagramType type)
	{
		if (type == zFormDiagram)
		{
			for (zItMeshHalfEdge he(oForm); !he.end(); he++)
			{
				int i = he.getId();

				if (form_tensionEdges[i]) he.getEdge().setColor(zMAGENTA);
				else if(form_ExternalForceEdges[i])  he.getEdge().setColor(zGREEN);
				else  he.getEdge().setColor(zBLUE);

				if (fixedVerticesBoolean.size() > 0)
				{
					if (fixedVerticesBoolean[he.getVertex().getId()] && fixedVerticesBoolean[he.getStartVertex().getId()])
						he.getEdge().setColor(zWHITE);
				}
			}
		}
		else if (type == zForceDiagram)
		{
			for (zItMeshHalfEdge he(oForce); !he.end(); he++)
			{
				int i = he.getId();	

				if (force_tensionEdges[i]) he.getEdge().setColor(zMAGENTA);
				else if (force_ExternalForceEdges[i])  he.getEdge().setColor(zGREEN);
				else  he.getEdge().setColor(zBLUE);
			}


		}
		else throw std::invalid_argument(" invalid diagram type.");
	}

	void setConstraints(zDiagramType type, const zIntArray& _fixedVertices = zIntArray())
	{


		if (type == zFormDiagram)
		{
			zFnMesh fnForm(oForm);

			if (_fixedVertices.size() == 0)
			{
				fixedVertices.clear();

				for (zItMeshVertex v(oForm); !v.end(); v++)
				{
					if (v.onBoundary()) fixedVertices.push_back(v.getId());
				}

			}
			else
			{
				fixedVertices = _fixedVertices;
			}

			fixedVerticesBoolean.clear();


			for (int i = 0; i < fnForm.numVertices(); i++)
			{
				fixedVerticesBoolean.push_back(false);
			}


			fnForm.setVertexColor(zColor(1, 1, 1, 1));

			for (int i = 0; i < fixedVertices.size(); i++)
			{
				zItMeshVertex v(oForm, fixedVertices[i]);
				zColor col;
				v.setColor(col);

				fixedVerticesBoolean[fixedVertices[i]] = true;
			}

			printf("\n fixed: %i ", fixedVertices.size());
		}

		else throw std::invalid_argument(" invalid diagram type.");

	}

	void setTensionEdges(zDiagramType type, const zIntArray& _tensionEdges = zIntArray())
	{

		 if (type == zFormDiagram)
		{
			 zFnMesh fnForm(oForm);
			form_tensionEdges.clear();

			form_tensionEdges.assign(fnForm.numHalfEdges(), false);

			for (int i = 0; i < _tensionEdges.size(); i++)
			{
				if (_tensionEdges[i] >= 0 && _tensionEdges[i] < fnForm.numHalfEdges())
				{
					form_tensionEdges[_tensionEdges[i]] = true;
				}
			}

			setElementColorDomain(type);
		}

		else if (type == zForceDiagram)
		{
			 zFnMesh fnForce(oForce);
			force_tensionEdges.clear();

			for (int i = 0; i < fnForce.numHalfEdges(); i++) force_tensionEdges.push_back(false);

			for (int i = 0; i < _tensionEdges.size(); i++)
			{
				if (_tensionEdges[i] >= 0 && _tensionEdges[i] < fnForce.numHalfEdges())
				{
					force_tensionEdges[_tensionEdges[i]] = true;
				}
			}

			setElementColorDomain(type);
		}

		else throw std::invalid_argument(" invalid diagram type.");

	}

	void setExternalForceEdges(zDiagramType type, const zIntArray& _extForceEdges = zIntArray())
	{

		if (type == zFormDiagram)
		{
			zFnMesh fnForm(oForm);
			form_ExternalForceEdges.clear();

			form_ExternalForceEdges.assign(fnForm.numHalfEdges(), false);

			for (int i = 0; i < _extForceEdges.size(); i++)
			{
				if (_extForceEdges[i] >= 0 && _extForceEdges[i] < fnForm.numHalfEdges())
				{
					form_ExternalForceEdges[_extForceEdges[i]] = true;
				}
			}

			setElementColorDomain(type);
		}

		else if (type == zForceDiagram)
		{
			zFnMesh fnForce(oForce);
			force_ExternalForceEdges.clear();

			for (int i = 0; i < fnForce.numHalfEdges(); i++) force_ExternalForceEdges.push_back(false);

			for (int i = 0; i < _extForceEdges.size(); i++)
			{
				if (_extForceEdges[i] >= 0 && _extForceEdges[i] < fnForce.numHalfEdges())
				{
					force_ExternalForceEdges[_extForceEdges[i]] = true;
				}
			}

			setElementColorDomain(type);
		}

		else throw std::invalid_argument(" invalid diagram type.");

	}

	void setVertexWeights(zDiagramType type, const zFloatArray& vWeights = zFloatArray())
	{
		if (type == zFormDiagram)
		{
			zFnMesh fnForm(oForm);

			if (vWeights.size() == 0)
			{
				formVWeights.clear();
				formVWeights.assign(fnForm.numVertices(), 1.0);
			}
			else
			{
				if (vWeights.size() != fnForm.numVertices()) throw std::invalid_argument("size of loads contatiner is not equal to number of form vertices.");

				formVWeights = vWeights;
			}

		}

		else if (type == zForceDiagram)
		{
			zFnMesh fnForce(oForce);

			if (vWeights.size() == 0)
			{
				forceVWeights.clear();
				forceVWeights.assign(fnForce.numVertices(), 1.0);

			}
			else
			{
				if (vWeights.size() != fnForce.numVertices()) throw std::invalid_argument("size of loads contatiner is not equal to number of force vertices.");

				forceVWeights = vWeights;
			}

		}
		
		else throw std::invalid_argument(" error: invalid zDiagramType type");
	}

	void setForceTensionEdgesfromForm()
	{
		zFnMesh fnForce(oForce);
		force_tensionEdges.clear();

		for (int i = 0; i < fnForce.numHalfEdges(); i++)
		{
			force_tensionEdges.push_back(false);
		}

		for (zItMeshHalfEdge he(oForm); !he.end(); he++)
		{
			int i = he.getId();
			if (formEdge_forceEdge[i] != -1 && form_tensionEdges[i])
			{
				force_tensionEdges[formEdge_forceEdge[i]] = true;;

				zItMeshHalfEdge e_force(oForce, formEdge_forceEdge[i]);

				int symEdge = e_force.getSym().getId();
				force_tensionEdges[symEdge] = true;;

			}
		}

		setElementColorDomain(zForceDiagram);
	}

	void setForceExternalEdgesfromForm()
	{
		zFnMesh fnForce(oForce);
		force_ExternalForceEdges.clear();

		for (int i = 0; i < fnForce.numHalfEdges(); i++)
		{
			force_ExternalForceEdges.push_back(false);
		}

		for (zItMeshHalfEdge he(oForm); !he.end(); he++)
		{
			int i = he.getId();
			if (formEdge_forceEdge[i] != -1 && form_ExternalForceEdges[i])
			{
				force_ExternalForceEdges[formEdge_forceEdge[i]] = true;;

				zItMeshHalfEdge e_force(oForce, formEdge_forceEdge[i]);

				int symEdge = e_force.getSym().getId();
				force_ExternalForceEdges[symEdge] = true;;

			}
		}

		setElementColorDomain(zForceDiagram);
	}

	void translateForceDiagram(float value)
	{
		zFnMesh fnForce(oForce);

		// bounding box
		zPoint minBB, maxBB;	
		fnForce.getBounds(minBB, maxBB);

		//zPoint Cen = fnForce.getCenter();
		
		zVector dir;
		dir.x = (maxBB.x - minBB.x);
		
		 dir *= value;
		 fnForce.setTranslation(dir, true);
	}

	//--- GET METHODS 

	bool getCorrespondingForceEdge(int formEdgeindex, zItMeshHalfEdge& outForceEdge)
	{
		if (formEdgeindex > formEdge_forceEdge.size()) throw std::invalid_argument(" error: index out of bounds.");
		if (formEdge_forceEdge[formEdgeindex] != -1) outForceEdge = zItMeshHalfEdge(oForce, formEdge_forceEdge[formEdgeindex]);

		return (formEdge_forceEdge[formEdgeindex] == -1) ? false : true;
	}

	bool getCorrespondingFormEdge(int forceEdgeindex, zItMeshHalfEdge& outFormEdge)
	{
		if (forceEdgeindex > forceEdge_formEdge.size()) throw std::invalid_argument(" error: index out of bounds.");

		if (forceEdge_formEdge[forceEdgeindex] != -1) outFormEdge = zItMeshHalfEdge(oForm, forceEdge_formEdge[forceEdgeindex]);

		return (forceEdge_formEdge[forceEdgeindex] == -1) ? false : true;
	}

	//---- UTILITY METHODS 
protected:

	void getHorizontalTargets(float formWeight)
	{
		targetEdges_form.clear();
		targetEdges_force.clear();

		zFnMesh fnForm(oForm);
		zFnMesh fnForce(oForce);

		targetEdges_force.assign(fnForce.numHalfEdges(), zVector());

		for (zItMeshHalfEdge e_form(oForm); !e_form.end(); e_form++)
		{
			int i = e_form.getId();

			//form edge
			int eId_form = e_form.getId();
			zVector e_form_vec = e_form.getVector();
			e_form_vec.normalize();


			if (formEdge_forceEdge[i] != -1)
			{
				// force edge
				int eId_force = formEdge_forceEdge[i];
				zItMeshHalfEdge e_force(oForce, eId_force);

				if (form_tensionEdges[eId_form])
				{
					eId_force = e_force.getSym().getId();
					e_force = e_force.getSym();
				}

				zVector e_force_vec = e_force.getVector();
				e_force_vec.normalize();

				// target edge 
				zVector e_target = (e_form_vec * formWeight) + (e_force_vec * (1 - formWeight));
				e_target.normalize();

				targetEdges_form.push_back(e_target);
				targetEdges_force[eId_force] = e_target;

			}

			else
			{
				// target edge 
				zVector e_target = (e_form_vec * 1);
				targetEdges_form.push_back(e_target);
			}


		}
	}

	bool checkHorizontalParallelity(zDomainFloat& deviation, float angleTolerance, bool colorEdges, bool printInfo)
	{
		bool out = true;
		vector<double> deviations;
		deviation.min = 10000;
		deviation.max = -10000;

		for (zItMeshHalfEdge e_form(oForm); !e_form.end(); e_form++)
		{


			//form edge
			int eId_form = e_form.getId();;
			zVector e_form_vec = e_form.getVector();
			e_form_vec.normalize();


			if (formEdge_forceEdge[eId_form] != -1)
			{
				// force edge
				int eId_force = formEdge_forceEdge[eId_form];
				zItMeshHalfEdge e_force(oForce, eId_force);

				zVector e_force_vec = e_force.getVector();
				e_force_vec.normalize();

				// angle

				double a_i = e_form_vec.angle(e_force_vec);
				if ((form_tensionEdges[eId_form]))a_i = 180 - a_i;

				deviations.push_back(a_i);

				if (a_i > angleTolerance)
				{
					out = false;
				}


				//printf("\n %i %i %i %i ", v1_form, v2_form, v1_force, v2_force);
				//printf("\n e: %i angle :  %1.2f ", i, a_i);

				if (a_i < deviation.min) deviation.min = a_i;
				if (a_i > deviation.max) deviation.max = a_i;

			}
			else
			{
				deviations.push_back(-1);
			}
		}


		if (printInfo)
		{
			printf("\n  tolerance : %1.4f minDeviation : %1.4f , maxDeviation: %1.4f ", angleTolerance, deviation.min, deviation.max);
		}

		if (colorEdges)
		{
			zDomainColor colDomain(zColor(180, 1, 1), zColor(0, 1, 1));

			for (zItMeshHalfEdge e_form(oForm); !e_form.end(); e_form++)
			{
				int i = e_form.getId();
				if (deviations[i] != -1)
				{
					zColor col = coreUtils.blendColor(deviations[i], deviation, colDomain, zHSV);

					if (deviations[i] < angleTolerance) col = zColor();

					e_form.getEdge().setColor(col);

					int eId_force = formEdge_forceEdge[i];

					zItMeshHalfEdge eForce(oForce, eId_force);
					eForce.getEdge().setColor(col);

				}

			}

		}


		return out;
	}

	void updateFormDiagram(float minmax_Edge, float dT, zIntergrationType type, int numIterations)
	{
		zFnMesh fnForm(oForm);

		if (fnFormParticles.size() != fnForm.numVertices())
		{
			fnFormParticles.clear();
			formParticlesObj.clear();


			for (int i = 0; i < fnForm.numVertices(); i++)
			{
				bool fixed = false;


				zObjParticle p;
				p.particle = zParticle(oForm.mesh.vertexPositions[i], fixed);
				formParticlesObj.push_back(p);

			}

			for (int i = 0; i < formParticlesObj.size(); i++)
			{
				fnFormParticles.push_back(zFnParticle(formParticlesObj[i]));
			}
		}

		vector<zVector> v_residual;
		v_residual.assign(fnForm.numVertices(), zVector());

		vector<double> edgelengths;
		fnForm.getEdgeLengths(edgelengths);

		double minEdgeLength, maxEdgeLength;
		minEdgeLength = coreUtils.zMin(edgelengths);
		maxEdgeLength = coreUtils.zMax(edgelengths);

		minEdgeLength = maxEdgeLength * minmax_Edge;


		zVector* positions = fnForm.getRawVertexPositions();

		for (int k = 0; k < numIterations; k++)
		{
			for (zItMeshVertex v(oForm); !v.end(); v++)
			{
				int i = v.getId();
				vector<zItMeshHalfEdge> cEdges;
				v.getConnectedHalfEdges(cEdges);

				zVector v_i = positions[v.getId()];

				// compute barycenter per vertex
				zVector b_i(0, 0, 0);
				for (auto& he : cEdges)
				{

					zVector v_j = positions[he.getVertex().getId()];

					zVector e_ij = v_i - v_j;
					double len_e_ij = e_ij.length();

					if (len_e_ij < minEdgeLength) len_e_ij = minEdgeLength;
					if (len_e_ij > maxEdgeLength) len_e_ij = maxEdgeLength;

					zVector t_ij = targetEdges_form[he.getSym().getId()];
					//zVector t_ij = targetEdges_form[he.getId()]; //// CHECK OTHER INSTANCES THAN HK BRIDGE
					t_ij.normalize();

					b_i += (v_j + (t_ij * len_e_ij));

				}


				b_i /= cEdges.size();

				// compute residue force				
				v_residual[i] = (b_i - v_i);

				////// HK BRidge remove z and x componenents
				//v_residual[i].x = 0;
				//v_residual[i].z = 0;

				zVector forceV = v_residual[i] * formVWeights[i];
				if (formVWeights[i] == 0.0) forceV = zVector();

				//forceV = zVector(0, 0, forceV.z);

				fnFormParticles[i].addForce(forceV);
			}

			//HK Bridge Min Edge length
			float minEdgeLen = 5.0;
			/*for (zItMeshEdge e(*formObj); !e.end(); e++)
			{
				int i = e.getId();

				if (e.getLength() < minEdgeLen)
				{

					zItMeshHalfEdge he0 = e.getHalfEdge(0);
					zItMeshHalfEdge he1 = e.getHalfEdge(1);

					zVector  he0_vec = he0.getVector();
					zVector  he1_vec = he1.getVector();
					he0_vec.normalize();
					he1_vec.normalize();

					zVector pForce1 = (he1.getStartVertex().getPosition() + he1_vec * minEdgeLen) - he1.getVertex().getPosition();
					if (formVWeights[he1.getVertex().getId()] == 0.0) pForce1 = zVector();
					fnFormParticles[he1.getVertex().getId()].addForce(pForce1);


					zVector pForce0 = (he0.getStartVertex().getPosition() + he0_vec * minEdgeLen) - he0.getVertex().getPosition();
					if (formVWeights[he0.getVertex().getId()] == 0.0) pForce0 = zVector();
					fnFormParticles[he0.getVertex().getId()].addForce(pForce0);
				}
			}*/

			// update positions
			for (int i = 0; i < fnFormParticles.size(); i++)
			{
				fnFormParticles[i].integrateForces(dT, type);
				fnFormParticles[i].updateParticle(true);
			}
		}

	}

	void updateForceDiagram(float minmax_Edge, float dT, zIntergrationType type, int numIterations)
	{
		zFnMesh fnForce(oForce);

		if (fnForceParticles.size() != fnForce.numVertices())
		{
			fnForceParticles.clear();
			forceParticlesObj.clear();


			for (int i = 0; i < fnForce.numVertices(); i++)
			{
				bool fixed = false;


				zObjParticle p;
				p.particle = zParticle(oForce.mesh.vertexPositions[i], fixed);
				forceParticlesObj.push_back(p);

			}

			for (int i = 0; i < forceParticlesObj.size(); i++)
			{
				fnForceParticles.push_back(zFnParticle(forceParticlesObj[i]));
			}
		}

		vector<zVector> v_residual;
		v_residual.assign(fnForce.numVertices(), zVector());

		vector<double> edgelengths;
		fnForce.getEdgeLengths(edgelengths);

		double minEdgeLength, maxEdgeLength;
		minEdgeLength = coreUtils.zMin(edgelengths);
		maxEdgeLength = coreUtils.zMax(edgelengths);

		minEdgeLength = maxEdgeLength * minmax_Edge;



		zVector* positions = fnForce.getRawVertexPositions();

		for (int k = 0; k < numIterations; k++)
		{
			for (zItMeshVertex v(oForce); !v.end(); v++)
			{
				int i = v.getId();
				vector<zItMeshHalfEdge> cEdges;
				cEdges.clear();
				v.getConnectedHalfEdges(cEdges);

				zVector v_i = positions[v.getId()];

				// compute barycenter per vertex
				zVector b_i(0, 0, 0);
				for (auto& he : cEdges)
				{
					zVector v_j = positions[he.getVertex().getId()];

					zVector e_ij = v_i - v_j;
					double len_e_ij = e_ij.length();

					if (len_e_ij < minEdgeLength) len_e_ij = minEdgeLength;
					if (len_e_ij > maxEdgeLength) len_e_ij = maxEdgeLength;

					zVector t_ij = targetEdges_force[he.getSym().getId()];
					t_ij.normalize();

					b_i += (v_j + (t_ij * len_e_ij));

				}

				b_i /= cEdges.size();

				// compute residue force
				v_residual[i] = b_i - v_i;

				zVector forceV = v_residual[i] * forceVWeights[i];

				//forceV = zVector(0, 0, forceV.z);

				fnForceParticles[i].addForce(forceV);
			}

			// update positions
			for (int i = 0; i < fnForceParticles.size(); i++)
			{
				fnForceParticles[i].integrateForces(dT, type);
				fnForceParticles[i].updateParticle(true);
			}
		}
	}
};

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool display = true;

bool c_form = false;
bool c_force = false;

bool d_ForceDiagram = true;
bool d_FormDiagram = true;
bool equilibrium = false;

bool d_Labels = true;

bool rotate90 = true;
bool computeHE_targets = true;

bool exportFiles = false;

bool eqReached = false;
zDomainFloat eq_deviations;



double background = 0.35;

double formWeight = 1.0;
double angleTolerance = 0.001;
double forceTolerance = 0.001;

double form_minMax = 0.3;
double force_minMax = 0.01;
double forceDiagramScale = 1.0;

double dT = 0.5;
zIntergrationType intType = zRK4;

string path = "data/2DGS/formMesh_arch_1.json";


////// --- zSpace Objects --------------------------------------------------
/*!<model*/
zModel model;

/*!<Objects*/

zTs2DGS myGS;




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
	

	
		
	//////////////////////////////////////////////////////////  DISPLAY SETUP
	// append to model for displaying the object
	model.addObject(myGS.oForm);
	model.addObject(myGS.oForce);

	// set display element booleans
	myGS.oForm.setDisplayElements(false, true, false);
	myGS.oForce.setDisplayElements(false, true, false);

	////////////////////////////////////////////////////////////////////////// Sliders

	int counter = 0;

	S = *new SliderGroup();
	
	S.addSlider(&background, "background");
	S.sliders[0].attachToVariable(&background, 0, 1);

	S.addSlider(&formWeight, "formWeight");
	S.sliders[1].attachToVariable(&formWeight, 0, 1);

	S.addSlider(&angleTolerance, "angleTolerance");
	S.sliders[2].attachToVariable(&angleTolerance, 0, 10);

	S.addSlider(&forceDiagramScale, "forceScale");
	S.sliders[3].attachToVariable(&forceDiagramScale, 0.0, 10.0);

	

	////////////////////////////////////////////////////////////////////////// Buttons

	B = *new ButtonGroup(Alice::vec(50, 450, 0));

	counter = 0; 
	B.addButton(&compute, "compute");
	B.buttons[++counter].attachToVariable(&compute);

	B.addButton(&d_FormDiagram, "d_FormDiagram");
	B.buttons[++counter].attachToVariable(&d_FormDiagram);

	B.addButton(&d_ForceDiagram, "d_ForceDiagram");
	B.buttons[++counter].attachToVariable(&d_ForceDiagram);

	B.addButton(&d_Labels, "d_Labels");
	B.buttons[++counter].attachToVariable(&d_Labels);

	B.addButton(&exportFiles, "exportFiles");
	B.buttons[++counter].attachToVariable(&exportFiles);

}

void update(int value)
{
	myGS.oForm.setDisplayObject(d_FormDiagram);
	myGS.oForce.setDisplayObject(d_ForceDiagram);

	zFnMesh fnForm(myGS.oForm);
	zFnMesh fnForce(myGS.oForce);

	zFloat4 scale = { forceDiagramScale,forceDiagramScale,forceDiagramScale,1 };
	fnForce.setScale(scale);

	if (c_form)
	{
		eqReached = false;
		myGS.createfromFile(path, zJSON, zFormDiagram);


		zFnMesh fnForm(myGS.oForm);

		// External Force 
		//zIntArray extForceEdgePairs = {10,16, 15,4, 17,12, 7,5, 18,14, 6,0, 3,8 };
		//zIntArray extForceEdgePairs = { 6,13, 1,14, 8,15, 2,16, 10,17, 3,18, 0,12 };

		//zIntArray extForceEdgePairs = { 0,17,14,18,12,19,15,20,13,21,16,22,3,23 }; // lenticular
		zIntArray extForceEdgePairs = { 0,12,9,13,7,14,10,15,8,16,11,17,1,18 }; // arch
		//zIntArray extForceEdgePairs = { 0,12,7,13,4,14,9,15,5,16,11,17,3,18 }; // tension bottom chord
		//zIntArray extForceEdgePairs = { 3,13,4,5,2,6,8,14,10,15,12,16,17,22,20,30,21,23,25,31,27,32,29,33,34,35 }; // tension bottom chord 2
		zIntArray extForceEdges;

		for (int i = 0; i < extForceEdgePairs.size(); i+= 2)
		{
			zItMeshHalfEdge he;
			if (fnForm.halfEdgeExists(extForceEdgePairs[i], extForceEdgePairs[i + 1], he))
			{
				extForceEdges.push_back(he.getId());
				extForceEdges.push_back(he.getSym().getId());

			}
		}

		myGS.setExternalForceEdges(zFormDiagram, extForceEdges);

		// Tension Edges 
		//zIntArray tensionEdgePairs = { 0,9, 9,1, 1,11, 11,2, 2,13, 13,3 };
		//zIntArray tensionEdgePairs = { 0,7, 7,4, 4,9, 9,5, 5,11, 11,3, 7,6, 4,1, 9,8, 5,2, 11,10 };


		//zIntArray tensionEdgePairs = { 0, 6, 1, 6, 1, 8, 2, 8, 2, 10, 3, 10 }; //lenticular
		zIntArray tensionEdgePairs = { 0,4,2,4,2,5,3,5,3,6,1,6,2,7,3,8,4,9,5,10,6,11 }; //arch
		//zIntArray tensionEdgePairs = { 0,6,1,6,1,8,2,8,2,10,3,10 }; //tension bottom chord
		//zIntArray tensionEdgePairs = { 0,7,0,9,1,9,1,11,2,11,17,24,18,24,18,26,19,26,19,28,28,35,7,35 }; //tension bottom chord 2
		zIntArray tensionHalfEdges;

		for (int i = 0; i < tensionEdgePairs.size(); i += 2)
		{
			zItMeshHalfEdge he;
			if (fnForm.halfEdgeExists(tensionEdgePairs[i], tensionEdgePairs[i + 1], he))
			{
				tensionHalfEdges.push_back(he.getId());
				tensionHalfEdges.push_back(he.getSym().getId());

			}
		}
		myGS.setTensionEdges(zFormDiagram, tensionHalfEdges);

		c_form = !c_form;
	}

	if (c_force)
	{
		myGS.createForceFromForm(rotate90);
		//myGS.createForceFromForm(false);

		myGS.translateForceDiagram(10);
		

		//alignForceDiagram_bottom(myVault, formObj, forceObj, form_v0, form_v1);

		c_force = !c_force;
	}

	if (equilibrium)
	{
		

		bool out = myGS.equilibrium(computeHE_targets, formWeight, dT, intType, 1, angleTolerance, form_minMax, force_minMax, eq_deviations, true, true);

		if (out)
		{
			eqReached = true;
			printf("\n equilibriumREACHED");
		}
		equilibrium = !equilibrium;
	}

	if (exportFiles)
	{
		
		fnForm.to("data/2DGS/out_formMesh.obj", zOBJ);		
		fnForce.to("data/2DGS/out_forceMesh.obj", zOBJ);

		exportFiles = !exportFiles;
	}
	
}

void draw()
{
	backGround(background);
	//drawGrid(50);

	S.draw();
	B.draw();

	model.draw();
		


	if(d_Labels)
	{
		for (zItMeshHalfEdge he(myGS.oForce); !he.end(); he++)
		{
			zPoint eCen_force = he.getCenter();
			string heForceID = to_string(he.getEdge().getId());

			zItMeshHalfEdge heForm;
			if (myGS.getCorrespondingFormEdge(he.getId(), heForm))
			{
				zPoint eCen_form = heForm.getCenter();
				
				model.displayUtils.drawTextAtPoint(heForceID, eCen_form);
				
				model.displayUtils.drawTextAtPoint(heForceID, eCen_force);

			}
			
			

			he++;
		}
	}


	//////////////////////////////////////////////////////////

	setup2d();
	glColor3f(0, 0, 0);

	string eQ = (eqReached) ? " Reached" : " Not Reached";
	drawString("Equilibrium:"  + eQ, vec(winW - 350, winH - 800, 0));
	drawString("Tolerance #:" + to_string(angleTolerance), vec(winW - 350, winH - 775, 0));
	drawString("Deviations #:" + to_string(eq_deviations.min) + " | " + to_string(eq_deviations.max), vec(winW - 350, winH - 700, 0));

	//string frameText = (frameCHECKS) ? "TRUE" : "FALSE";
	//drawString("Frame CHECKS:" + frameText, vec(winW - 350, winH - 750, 0));



	drawString("KEY Press", vec(winW - 350, winH - 650, 0));

	drawString("r - create Form - Force Diagrams", vec(winW - 350, winH - 625, 0));
	drawString("p - compute Equilibrium", vec(winW - 350, winH - 600, 0));
	drawString("o - Compute targets", vec(winW - 350, winH - 575, 0));
	

	drawString("e - export Meshes", vec(winW - 350, winH - 425, 0));

	
	restore3d();

}

void keyPress(unsigned char k, int xm, int ym)
{
	if (k == 'r')
	{
		c_form = true;
		c_force = true;
	}

	if (k == 'p')
	{
		equilibrium = true;;
	}

	if (k == 'c')
	{
		computeHE_targets = true;
	}
}

void mousePress(int b, int s, int x, int y)
{
	
}

void mouseMotion(int x, int y)
{	

}



#endif // _MAIN_
