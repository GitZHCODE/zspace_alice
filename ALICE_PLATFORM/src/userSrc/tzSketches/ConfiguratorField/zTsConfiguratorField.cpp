// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Taizhong Chen <taizhong.chen@zaha-hadid.com>
//


#include <userSrc/tzSketches/ConfiguratorField/zTsConfiguratorField.h>

namespace zSpace
{

	std::vector<zVector> computeCirclePoints(zVector& center, double radius, int numPoints) {
		std::vector<zVector> circlePoints;
		circlePoints.reserve(numPoints); // Reserve memory for efficiency

		const double TWO_PI = 2.0 * M_PI;
		double angleIncrement = TWO_PI / numPoints;

		for (int i = 0; i < numPoints; ++i) {
			double theta = i * angleIncrement;
			double x = center.x + radius * std::cos(theta);
			double y = center.y + radius * std::sin(theta);
			circlePoints.push_back(zVector(x,y,0));
		}

		return circlePoints;
	}

	/* PUBLIC */

	Parcel::Parcel() {};
	Parcel::Parcel(zPoint _centre, int _parcelId, float _radius)
	{
		centre = _centre;
		parcelId = _parcelId;
		radius = _radius;
	};
	Parcel::~Parcel() {};

	/* PUBLIC */

	zTsConfiguratorField::zTsConfiguratorField() :gen(std::random_device{}()) {};
	zTsConfiguratorField::~zTsConfiguratorField() {};

	void zTsConfiguratorField::initialise(
		string path_graphMesh,
		string path_underlayMesh,
		int fieldNumX,
		int fieldNumY,
		float minNum,
		float maxNum,
		float minRadius,
		float maxRadius
	)
	{
		

		initialise_graphMesh(path_graphMesh);
		initialise_underlayMesh(path_underlayMesh);
		initialise_field(fieldNumX, fieldNumY);
		intialise_parcels(minNum, maxNum, minRadius, maxRadius);
	}

	void zTsConfiguratorField::compute(float parcelRotation, float parcelOffset)
	{
		equiliseParcels();
		rotateParcels(parcelRotation);
		markParcels(parcelOffset);
	}

	void zTsConfiguratorField::getVectorField(zPointArray& positions, zVectorArray& vectors)
	{
		//zFnMeshVectorField fnField(vectorField);
		//fnField.getPositions(positions);
		//fnField.getFieldValues(vectors);

		fnField.getPositions(positions);
		vectors = fnField.getGradients(EPS);

		for (auto& vec : vectors)
		{
			vec.normalize();

			//if (vec.length() > EPS)
			//	vec.normalize();
			//else
			//	vec = zVector(0, 0, 0);
		}
	}

	void zTsConfiguratorField::draw(bool d_graphMesh, bool d_fieldMesh, bool d_parcels, bool d_parcelBounds)
	{

		if (d_graphMesh)
		{
			//graphMesh.setDisplayElements(false, true, false);
			//graphMesh.draw();

			//medialGraph.setDisplayElements(false, true);
			//medialGraph.draw();

			for (zItGraphEdge e(medialGraph); !e.end(); e++)
			{
				zPointArray pos;
				e.getVertexPositions(pos);
				display.drawLine(pos[0], pos[1], zRED, 4);
			}
		}

		if (d_fieldMesh)
		{
			double vecLength = 0.4;

			//scalarField.setDisplayElements(false, true, true);
			//scalarField.draw();

			zPointArray pos;
			zVectorArray vecs;
			getVectorField(pos, vecs);

			for (int i = 0; i < pos.size(); i++)
				display.drawLine(pos[i], pos[i] + vecs[i] * vecLength, zGREY, 1);

		}

		if (d_parcels)
		{
			double vecLength = 0.2;

			for (auto& parcel : parcels)
			{
				if (!parcel.isTooClose)
				{
					display.drawPoint(parcel.centre, zRED, 2);
					display.drawLine(parcel.centre, parcel.centre + parcel.vec * parcel.radius, zBLACK, 2);
					//display.drawLine(parcel.centre, parcel.centre + parcel.vec * vecLength, zWHITE, 2);
				}
			}
		}

		if (d_parcelBounds)
		{
			for (auto& parcel : parcels)
			{
				if (!parcel.isTooClose)
				{
					vector<zVector> circlePts = computeCirclePoints(parcel.centre, parcel.radius, 20);
					display.drawPoint(parcel.centre, zRED, 2);
					display.drawCircle(parcel.centre, circlePts, true, parcel.col, 2);
				}
			}
		}
	}

	/* PRIVATE */

	void zTsConfiguratorField::initialise_graphMesh(string path)
	{
		zFnMesh fnMesh(graphMesh);
		fnMesh.from(path, zJSON);
	}

	void zTsConfiguratorField::initialise_underlayMesh(string path)
	{
		zFnMesh fnMesh(underlayMesh);
		fnMesh.from(path, zJSON);
	}

	void zTsConfiguratorField::initialise_field(int numX, int numY, float fieldOffset)
	{
		fnField = zFnMeshScalarField(scalarField);

		zPoint minBB, maxBB;
		graphMesh.getBounds(minBB, maxBB);

		float extVal = 2.0f;
		minBB.x -= extVal;
		minBB.y -= extVal;
		maxBB.x += extVal;
		maxBB.y += extVal;

		fnField.create(minBB, maxBB, numX, numY, 1, true, false);

		//setFieldValuesFromMesh(graphMesh);
		//setFieldValueFromMesh_medial(graphMesh);

		setFieldValuesFromMesh_medial(graphMesh);
		//setFieldValuesFromMesh(graphMesh);
		//computeVectorField();

		fieldVectors = fnField.getGradients(EPS);
		for (auto& vec : fieldVectors)
			vec.normalize();
	}

	void zTsConfiguratorField::intialise_parcels(float minNum, float maxNum, float minRadius, float maxRadius)
	{
		parcels.clear();

		zUtilsCore core;

		int numParcels = 0;

		float min = 0;
		float max = 360;
		zDomainFloat inDomain(min, max);
		zDomainFloat outDomain_num(minNum, maxNum);
		zDomainFloat outDomain_rad(maxRadius, minRadius);

		float rndMin = -0.1;
		float rdnMax = 0.1;

		zItMeshFace f(graphMesh);
		for (; !f.end(); f++)
		{
			float nf_parcels = core.ofMap(f.getColor().h, inDomain, outDomain_num);
			float radius = core.ofMap(f.getColor().h, inDomain, outDomain_rad);
			zColor colour = f.getColor();
			zVector fCentre = f.getCenter();

			for (int i = 0; i < nf_parcels; i++)
			{
				zItMeshScalarField s(scalarField, fCentre);

				parcels.emplace_back(f.getCenter() + randomNumber(rndMin, rdnMax), numParcels, radius);
				parcels[numParcels].vec = fieldVectors[s.getId()];
				parcels[numParcels].col = colour;
				numParcels++;
			}
			//cout << "f_parcels: " << nf_parcels << endl;
			//cout << "f_radius: " << radius << endl;
		}

		cout << "numParcels: " << numParcels << endl;
	}

	// field

	void zTsConfiguratorField::setFieldScalar(double _min, double _max)
	{
		zUtilsCore core;
		zFloatArray fVals;
		fnField.getFieldValues(fVals);

		float min = core.zMin(fVals);
		float max = core.zMax(fVals);
		zDomainFloat inDomain(min, max);
		zDomainFloat outDomain(_min, _max);
		for (auto& val : fVals)
		{
			val = core.ofMap(val, inDomain, outDomain);
		}

		fnField.setFieldValues(fVals, zFieldColorType::zFieldRegular);
		fnField.updateColors();
	}

	void zTsConfiguratorField::setFieldValuesFromMesh(zObjMesh& oMesh)
	{

		// setup polygons
		zFnMesh fnMesh(graphMesh);
		vector<zPointArray> polys;
		zColorArray fCols;
		fnMesh.getFaceColors(fCols);

		zItMeshFace f(graphMesh);
		for (; !f.end(); f++)
		{
			zPointArray fVertices;
			f.getVertexPositions(fVertices);
			polys.push_back(fVertices);
		}

		// compute field values
		zUtilsCore core;

		zPointArray positions;
		//fnField.getPositions(positions);
		fnField.fnMesh.getVertexPositions(positions);

		zFloatArray vals;
		vals.assign(positions.size(), 0.0f);

		for (int i = 0; i < positions.size(); i++)
		{
			for (int j = 0; j < polys.size(); j++)
			{
				bool isInside = core.pointInPlanarPolygon(positions[i], polys[j], zVector(0, 0, 1));
				if (isInside)
				{
					vals[i] = fCols[j].h;
					break;
				}
			}
		}

		LOG_DEV << "vals.size(): " << vals.size() << endl;
		LOG_DEV << "fnField.numFieldValues(): " << fnField.numFieldValues() << endl;
		//for (auto& v : vals)
		//	LOG_DEV << v << endl;

		fnField.setFieldColorDomain(zDomainColor(zColor(0, 1, 1), zColor(180, 1, 1)));

		LOG_DEV << "v before smooth: " << endl;
		for (auto& v : vals)
			LOG_DEV << "v: " << v << endl;
		LOG_DEV << "--------------" << endl;

		fnField.smoothField(vals, 3);
		fnField.setFieldValues(vals, zFieldColorType::zFieldRegular);
		setFieldScalar(0.0, 1.0);
		//fnField.updateColors(zFieldColorType::zFieldSDF);

		//vals.clear();
		//fnField.getFieldValues(vals);
		//LOG_DEV << "v after smooth: " << endl;
		//for (auto& v : vals)
		//	LOG_DEV << "v: " << v << endl;
		//LOG_DEV << "--------------" << endl;

	}

	void zTsConfiguratorField::setFieldValuesFromMesh_medial(zObjMesh& oMesh)
	{
		zItMeshHalfEdge he_start;
		zUtilsCore core;
		// find a star point
		for (zItMeshVertex v(oMesh); !v.end(); v++)
		{
			// find the entry halfedge
			if (v.checkValency(6))
			{
				zIntArray cHes;
				v.getConnectedHalfEdges(cHes);

				for (auto& id : cHes)
				{
					zItMeshHalfEdge he(oMesh, id);
					bool found = true;
					do
					{
						if (he.getVertex().onBoundary())
						{
							found = false;
							break;
						}
						else
							he = he.getNext().getSym().getNext();
					} while (!he.getVertex().checkValency(6));

					if (found)
					{
						do 
						{
							he = he.getNext().getSym().getNext();
						} while (!he.getVertex().onBoundary());
						he_start = he.getSym();
						break;
					}
				}
			}
		}

		vector<int> visitedEdges;

		// walk and get all medial hes
		vector<zVector> positions;
		vector<int> pConnects;

		//positions.push_back(he_start.getStartVertex().getPosition());
		//positions.push_back(he_start.getVertex().getPosition());
		//pConnects.push_back(0);
		//pConnects.push_back(1);

		//zFnGraph fnGraph(medialGraph);
		//fnGraph.create(positions, pConnects);

		int id_start = he_start.getId();
		do
		{
			int edgeId = he_start.getEdge().getId();
			int temp;
			if (!core.checkRepeatElement(edgeId, visitedEdges, temp))
			{
				zVector pt_start = he_start.getStartVertex().getPosition();
				zVector pt_end = he_start.getVertex().getPosition();

				int id_start, id_end;
				bool chk_start = core.checkRepeatVector(pt_start, positions, id_start);
				bool chk_end = core.checkRepeatVector(pt_end, positions, id_end);

				if (!chk_start)
				{
					positions.push_back(pt_start);
					id_start = positions.size() - 1;
				}
				if (!chk_end)
				{
					positions.push_back(pt_end);
					id_end = positions.size() - 1;
				}

				pConnects.push_back(id_start);
				pConnects.push_back(id_end);

				visitedEdges.push_back(edgeId);
			}

			if (he_start.getVertex().onBoundary())
				he_start = he_start.getSym();
			else
				he_start = he_start.getNext().getSym().getNext();
		} while (he_start.getId() != id_start);

		zFnGraph fnGraph(medialGraph);
		fnGraph.create(positions, pConnects);

		// extend the graph
		float extVal = 0.5;
		zPoint* pos = fnGraph.getRawVertexPositions();
		for(zItGraphVertex v(medialGraph);!v.end();v++)
		{
			if (v.checkValency(1))
				pos[v.getId()] = pos[v.getId()] + v.getHalfEdge().getVector() * extVal * -1;
		}

		zScalarArray scalars;
		fnField.getScalarsAsEdgeDistance(scalars,medialGraph, 0.1f, true);
		
		fnField.smoothField(scalars, 3);
		fnField.setFieldValues(scalars, zFieldColorType::zFieldRegular);
		setFieldScalar(0.0, 1.0);
	}

	void zTsConfiguratorField::computeVectorField()
	{
		//fnField.updateColors(zFieldColorType::zFieldSDF);
	}


	// parcels

	void zTsConfiguratorField::registerParcels(int maxNumParcels)
	{

	}

	void zTsConfiguratorField::equiliseParcels()
	{
		vector<zVector> points;
		vector<float>radius;

		for (auto& parcel : parcels)
		{
			points.push_back(parcel.centre);
			radius.push_back(parcel.radius);
		}


		int numParticles = 0;
		vector<zVector> boundaries = getBoundaries(graphMesh);
		//boundaries = generateBoundaryParticles(boundaries, 0.2);
		
		makePointsEquiDistant(points, radius, boundaries);

		for (int i = 0; i < points.size(); i++)
		{
			parcels[i].centre = points[i];

			zItMeshScalarField s(scalarField, parcels[i].centre);
			parcels[i].vec = fieldVectors[s.getId()];
		}

	}

	void zTsConfiguratorField::markParcels(float threshold)
	{
		zUtilsCore core;

		for (auto& parcel : parcels)
		{
			parcel.isTooClose = false;
			double minDist = 10000;
			for (zItGraphEdge e(medialGraph); !e.end(); e++)
			{
				zPointArray pos;
				e.getVertexPositions(pos);
				{
					zPoint temp;
					double dist = core.minDist_Edge_Point(parcel.centre, pos[0], pos[1], temp);
					if (dist < minDist)
					{
						minDist = dist;
					}
				}
			}

			if (minDist < threshold)
				parcel.isTooClose = true;
		}
	}

	void zTsConfiguratorField::rotateParcels(float parcelRotation)
	{
		for (auto& parcel : parcels)
			parcel.vec = parcel.vec.rotateAboutAxis(zVector(0, 0, 1), parcelRotation);
	}

	// particles
	void zTsConfiguratorField::makePointsEquiDistant(vector<zVector>& points, const vector<zVector>& boundaryPolygon, float step)
	{
		size_t nPoints = points.size();
		std::vector<zVector> forces(nPoints, zVector(0.0f, 0.0f, 0.0f));

		// Calculate repulsive forces
		for (size_t i = 0; i < nPoints; i++) 
		{
			for (size_t j = 0; j < nPoints; j++) 
			{
				if (i == j) continue;

				zVector diff = points[j] - points[i];
				float distance = diff.length();

				if (distance > 1e-2f) 
				{
					diff.normalize();
					diff *= 1.0f / (distance * distance);
					forces[i] -= diff;
				}

				forces[i].z = 0;
			}
		}

		// Normalize forces
		normalizeForces(forces);

		// Apply forces to move points
		for (size_t i = 0; i < nPoints; i++) 
		{
			if (forces[i].length() < 1.0f) 
			{
				zVector newPosition = points[i] + forces[i] * step;

				// Check if new position is inside the boundary
				if (isPointInsidePolygon(boundaryPolygon, newPosition)) 
				{
					points[i] = newPosition;
				}
				else 
				{
					// Move back by twice the force if outside the boundary
					points[i] -= forces[i] * 2.0f * step;
				}
			}
		}
	}

	void zTsConfiguratorField::makePointsEquiDistant(
		vector<zVector>& points,
		const vector<float>& radius,
		const vector<zVector>& boundaryPolygon,
		float step)
	{
		int nPoints = points.size();
		std::vector<zVector> forces(nPoints, zVector(0.0f, 0.0f, 0.0f));

		// Calculate repulsive forces based on individual radii
		for (int i = 0; i < nPoints; i++)
		{
			for (int j = 0; j < nPoints; j++)
			{
				if (i == j) continue;

				zVector diff = points[j] - points[i];
				float distance = diff.length();

				// Desired minimum distance between points i and j
				float desiredMinDistance = radius[i] + radius[j];

				if (distance > 1e-6f)
				{
					float overlap = desiredMinDistance - distance;

					if (overlap > 0.0f)
					{
						// Points are too close; apply repulsive force
						diff.normalize();
						float forceMagnitude = overlap / distance; // Adjust force as needed
						diff *= forceMagnitude;
						forces[i] -= diff;
					}
				}
				else
				{
					// Points are at the same position; apply a strong repulsive force
					zVector randomDirection(
						static_cast<float>(rand()) / RAND_MAX - 0.5f,
						static_cast<float>(rand()) / RAND_MAX - 0.5f,
						0.0f);
					randomDirection.normalize();
					forces[i] += randomDirection * desiredMinDistance;
				}

				// Keep forces in the XY plane
				forces[i].z = 0.0f;
			}
		}

		// Normalize forces
		normalizeForces(forces);

		// Apply forces to move points
		for (int i = 0; i < nPoints; i++)
		{
			if (forces[i].length() < 1.0f)
			{
				zVector newPosition = points[i] + forces[i] * step;

				// Check if new position is inside the boundary
				if (isPointInsidePolygon(boundaryPolygon, newPosition))
				{
					points[i] = newPosition;
				}
				else
				{
					// Adjust movement to stay within the boundary
					points[i] -= forces[i] * 2.0f * step;
				}
			}
		}
	}

	void zTsConfiguratorField::normalizeForces(vector<zVector>& forces)
	{
		float force_min = 1e6f, force_max = -1e6f;

		// Find min and max force magnitudes
		for (auto& force : forces) 
		{
			float d = force.length();
			force_max = std::max(force_max, d);
			force_min = std::min(force_min, d);
		}

		// Rescale forces to be within 0 & 1
		for (auto& force : forces) 
		{
			float d = force.length();
			if (d > 1e-6f) 
			{
				force.normalize();
				float mappedValue = mapValue(d, force_min, force_max, 0.0f, 1.0f);
				force *= mappedValue;
			}
			else {
				force = zVector(0.0f, 0.0f, 0.0f);
			}
		}
	}

	bool zTsConfiguratorField::isPointInsidePolygon(const vector<zVector>& polygon, const zVector& point)
	{
		int count = 0;
		size_t n = polygon.size();
		for (size_t i = 0; i < n; i++) 
		{
			const zVector& a = polygon[i];
			const zVector& b = polygon[(i + 1) % n];

			if (((a.y > point.y) != (b.y > point.y)) &&
				(point.x < (b.x - a.x) * (point.y - a.y) / (b.y - a.y + 1e-6f) + a.x))
			{
				count++;
			}
		}
		return (count % 2) == 1;
	}


	// assets

	void zTsConfiguratorField::populateAssets()
	{

	}

	// generic

	float zTsConfiguratorField::randomNumber(float min, float max)
	{
		std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}

	vector<zVector> zTsConfiguratorField::getBoundaries(zObjMesh& oMesh)
	{
		vector<zVector> boundaries;

		zItMeshHalfEdge he(oMesh);
		zItMeshHalfEdge he_start;

		for (; !he.end(); he++)
		{
			if (he.onBoundary() && he.getStartVertex().checkValency(2))
			{
				he_start = he;
				break;
			}
		}

		int id_start = he_start.getId();

		do
		{
			boundaries.push_back(he_start.getStartVertex().getPosition());
			he_start = he_start.getNext();
		} while (he_start.getId() != id_start);

		return boundaries;
	}

	vector<zVector> zTsConfiguratorField::generateBoundaryParticles(vector<zVector>& boundaries, float dist)
	{
		vector<zVector> particles;

		if (boundaries.empty())
			return particles;

		int numVertices = boundaries.size();

		// Determine the number of segments
		int numSegments = boundaries[boundaries.size() - 1] == boundaries[0] ? numVertices : numVertices - 1;

		for (int i = 0; i < numSegments; ++i) {
			// Get the start and end points of the segment
			zVector& start = boundaries[i];
			zVector& end = boundaries[(i + 1) % numVertices]; // Use modulo for closed loop

			// Compute the direction vector and length of the segment
			zVector dir = end - start;
			float segmentLength = dir.length();

			if (segmentLength == 0.0f)
				continue; // Skip if the segment length is zero

			dir.normalize();

			// Determine the number of particles along this segment
			int numParticlesInSegment = static_cast<int>(segmentLength / dist);

			// Generate particles along the segment
			for (int j = 0; j <= numParticlesInSegment; ++j) {
				float t = (j * dist) / segmentLength;
				if (t > 1.0f)
					t = 1.0f; // Ensure t does not exceed 1.0

				zVector point = start + dir * t;
				particles.push_back(point);
			}
		}

		return particles;
	}

	float zTsConfiguratorField::mapValue(float value, float inMin, float inMax, float outMin, float outMax)
	{
		if (inMax - inMin == 0) return outMin;
		return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
	}
}
