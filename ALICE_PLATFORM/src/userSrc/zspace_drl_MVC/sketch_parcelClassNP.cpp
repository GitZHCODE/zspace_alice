//#define _MAIN_

#ifdef _MAIN_

#include "main.h"

#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

//#include <headers/include/zCore.h>
//#include <headers/include/zGeometry.h>
//#include <headers/include/zDisplay.h>
//#include <headers/include/zData.h>
//#include <headers/include/zIO.h> 
//
using namespace zSpace;

////////////////////////////////////////////////////////////////////////// General

bool compute = false;
bool computeMap = false;
bool display = true;
bool extrudeMesh = false;         // NEW: button flag
bool showColorHeightMesh = false; // NEW: button flag

double background = 0.35;
double heightMin = 1.0;           // NEW: extrusion domain
double heightMax = 30.0;          // NEW: extrusion domain

double weightBlend = 0.5;  // 0 = use parcel direction, 1 = use vector to boundary
double prevWeightBlend = -1.0;  // initial dummy


////////////////////////////////////////////////////////////////////////// zSpace Objects
zModel model;

zObjMesh oMesh;
zObjGraph oGraph;

//--------------------------------------------
// Utilities
//--------------------------------------------

Alice::vec zVecToAliceVec(const zSpace::zVector& in)
{
    return Alice::vec(in.x, in.y, in.z);
}

zTransform createRotationMatrixFromTo(zVector from, zVector to)
{
    from.normalize();
    to.normalize();

    float dot = from * to;
    zVector axis = from ^ to;
    axis.normalize();

    float angle = acos(dot);

    zTransform rot;
    rot.setIdentity();

    // Rodrigues' rotation formula
    float c = cos(angle);
    float s = sin(angle);
    float t = 1 - c;

    float x = axis.x, y = axis.y, z = axis.z;

    rot(0, 0) = t * x * x + c;
    rot(0, 1) = t * x * y - s * z;
    rot(0, 2) = t * x * z + s * y;

    rot(1, 0) = t * x * y + s * z;
    rot(1, 1) = t * y * y + c;
    rot(1, 2) = t * y * z - s * x;

    rot(2, 0) = t * x * z - s * y;
    rot(2, 1) = t * y * z + s * x;
    rot(2, 2) = t * z * z + c;

    return rot;
}

bool isInsideParcelPolygon(zVector testPt, zVector* poly, int N)
{
    int count = 0;
    for (int i = 0; i < N; i++)
    {
        zVector a = poly[i];
        zVector b = poly[(i + 1) % N];

        if (((a.y > testPt.y) != (b.y > testPt.y)) &&
            (testPt.x < (b.x - a.x) * (testPt.y - a.y) / (b.y - a.y + 1e-6f) + a.x))
        {
            count++;
        }
    }
    return (count % 2 == 1);
}

zVector zLerp(const zVector& a, const zVector& b, double t)
{
    return zVector(
        a.x * (1.0 - t) + b.x * t,
        a.y * (1.0 - t) + b.y * t,
        a.z * (1.0 - t) + b.z * t
    );
}

//--------------------------------------------
// Global Variables
//--------------------------------------------

enum LandUseType { RESIDENTIAL, COMMERCIAL, INDUSTRIAL, PARK };
double gNumPoints = 9;
double gMinDistance = 50;
bool instanceMeshes = false;  // button for instancing the mesh


//--------------------------------------------
// Parcel Class
//--------------------------------------------


class Parcel
{
public:
    zVector center;
    double radius;
    zVector direction;
    LandUseType landUse;
    std::vector<LandUseType> adjacentLandUses;
    zVector boundaryPoints[50];

    std::vector<zVector> interiorPoints; // NEW
    int numPoints = 12;                  // default value
    double minDistance = 50;             // default value

    std::vector<LandUseType> interiorPointLandUses; // <-- NEW


    void createParcel(zVector _center, double _radius, zVector _direction, LandUseType _landUse, std::vector<LandUseType> _adjacentLandUses)
    {
        center = _center;
        radius = _radius;
        direction = _direction;
        landUse = _landUse;
        adjacentLandUses = _adjacentLandUses;

        float inc = TWO_PI / 50.0;
        for (int i = 0; i < 50; i++)
        {
            float x = radius * cos(inc * i);
            float y = radius * sin(inc * i);
            boundaryPoints[i] = zVector(x, y, 0);
        }

        direction.normalize();
        zVector u = direction;
        zVector v = zVector(-u.y, u.x, 0);
        zVector w = zVector(0, 0, 1);

        zTransform TM;
        TM.setIdentity();
        TM.col(0) << u.x, u.y, u.z, 1;
        TM.col(1) << v.x, v.y, v.z, 1;
        TM.col(2) << w.x, w.y, w.z, 1;
        TM.col(3) << center.x, center.y, center.z, 1;

        for (int i = 0; i < 50; i++)
            boundaryPoints[i] = boundaryPoints[i] * TM;
    }

    // In Parcel class
    int numBoundaryPoints = 0;

    void createPolygonFromCircleParams(std::vector<float> paramValues)
    {
        int n = paramValues.size();
        if (n < 3) return; // Require at least 3 points to form a polygon

        for (int i = 0; i < n && i < 50; i++)
        {
            float angle = TWO_PI * paramValues[i];
            float x = radius * cos(angle);
            float y = radius * sin(angle);
            boundaryPoints[i] = zVector(x, y, 0);
        }

        numBoundaryPoints = n; // ✅ track the actual polygon size

        direction.normalize();
        zVector u = direction;
        zVector v = zVector(-u.y, u.x, 0);
        zVector w = zVector(0, 0, 1);

        zTransform TM;
        TM.setIdentity();
        TM.col(0) << u.x, u.y, u.z, 1;
        TM.col(1) << v.x, v.y, v.z, 1;
        TM.col(2) << w.x, w.y, w.z, 1;
        TM.col(3) << center.x, center.y, center.z, 1;

        for (int i = 0; i < n && i < 50; i++)
        {
            boundaryPoints[i] = boundaryPoints[i] * TM;
        }
    }

    // Update displayParcel()
    void displayParcel()
    {
        glColor3f(0, 0, 0);
        for (int i = 0; i < numBoundaryPoints; i++)
        {
            zVector start = boundaryPoints[i];
            zVector end = boundaryPoints[(i + 1) % numBoundaryPoints];
            drawLine(zVecToAliceVec(start), zVecToAliceVec(end));
        }

        glPointSize(8);
        for (int i = 0; i < interiorPoints.size(); i++)
        {
            switch (interiorPointLandUses[i])
            {
            case RESIDENTIAL: glColor3f(0.2, 0.2, 1); break;
            case COMMERCIAL: glColor3f(1, 0, 0); break;
            case INDUSTRIAL: glColor3f(0.5, 0.5, 0.5); break;
            case PARK: glColor3f(0, 1, 0); break;
            }
            drawPoint(zVecToAliceVec(interiorPoints[i]));
        }
        glPointSize(1);
    }


    void generateInteriorPoints()
    {
        interiorPoints.clear();
        interiorPointLandUses.clear(); // clear landuse array too

        int attempts = 0;

        while (interiorPoints.size() < numPoints && attempts < numPoints * 100)
{
    zVector pt(ofRandom(-radius, radius), ofRandom(-radius, radius), 0);

    if (!isInsideParcelPolygon(pt, boundaryPoints, numBoundaryPoints)) { attempts++; continue; }

    bool ok = true;
    for (auto& p : interiorPoints)
    {
        if (p.distanceTo(pt) < minDistance)
        {
            ok = false;
            break;
        }
    }

    if (ok) interiorPoints.push_back(pt);
    attempts++;
}


        // Transform points
        zVector u = direction;
        zVector v = zVector(-u.y, u.x, 0);
        zVector w = zVector(0, 0, 1);

        zTransform TM;
        TM.setIdentity();
        TM.col(0) << u.x, u.y, u.z, 1;
        TM.col(1) << v.x, v.y, v.z, 1;
        TM.col(2) << w.x, w.y, w.z, 1;
        TM.col(3) << center.x, center.y, center.z, 1;

        for (auto& p : interiorPoints)
            p = p * TM;

        assignLandUseToPoints(); // assign land use after positioning
    }

    void assignLandUseToPoints()
    {
        interiorPointLandUses.clear();

        for (auto& p : interiorPoints)
        {
            double minDist = 1e10;
            LandUseType closestType = landUse; // default to parcel's own landuse

            for (auto adj : adjacentLandUses)
            {
                double d = 0;
                switch (adj)
                {
                case COMMERCIAL: d = p.distanceTo(center + zVector(20, 0, 0)); break;
                case PARK:       d = p.distanceTo(center + zVector(0, 20, 0)); break;
                case INDUSTRIAL: d = p.distanceTo(center + zVector(-20, 0, 0)); break;
                case RESIDENTIAL:d = p.distanceTo(center + zVector(0, -20, 0)); break;
                }

                if (d < minDist)
                {
                    minDist = d;
                    closestType = adj;
                }
            }


            // If the closest landuse is PARK, override it to RESIDENTIAL
            if (closestType == PARK)
                closestType = RESIDENTIAL;

            interiorPointLandUses.push_back(closestType);
        }
    }

    zVector getVectorToClosestBoundaryPoint(zVector pt)
    {
        float minDist = 1e10;
        zVector closest;
        for (int i = 0; i < numBoundaryPoints; i++)
        {
            float d = pt.distanceTo(boundaryPoints[i]);
            if (d < minDist)
            {
                minDist = d;
                closest = boundaryPoints[i];
            }
        }
        return closest - pt;  // vector pointing from point to boundary
    }


};

//-----------Import mesh and instance it-------//

class ParcelMeshManager
{
public:
    std::map<LandUseType, zObjMesh> baseMeshes; // Store mesh for each landuse type
    zModel* modelPtr = nullptr;                 // Pointer to main zModel (for adding instances)
    zObjMesh* heightFieldObj = nullptr;   // ✅ add this
    zFnMesh heightFieldFn;

    void setup(zModel* _model)
    {
        modelPtr = _model;
    }

    void loadMesh(LandUseType type, std::string filepath)
    {
        zObjMesh obj;
        zFnMesh fnMesh(obj);
        fnMesh.from(filepath, zOBJ);
        baseMeshes[type] = obj;
    }

    void loadHeightField(std::string jsonPath) // Load the Height field mesh
    {
        if (heightFieldObj != nullptr)
        {
            delete heightFieldObj; // ✅ remove previous
            heightFieldObj = nullptr;
        }

        heightFieldObj = new zObjMesh();  // fresh allocation
        heightFieldFn = zFnMesh(*heightFieldObj);
        heightFieldFn.from(jsonPath, zJSON);
    }

    float sampleHeightAtPoint(zVector pt) // Sample the height based on the color of the mesh points
    {
        zPointArray verts;
        zColorArray colors;
        heightFieldFn.getVertexPositions(verts);
        heightFieldFn.getVertexColors(colors);

        float minDist = 1e10;
        int closestID = -1;

        for (int i = 0; i < verts.size(); i++)
        {
            float d = pt.distanceTo(verts[i]);
            if (d < minDist)
            {
                minDist = d;
                closestID = i;
            }
        }

        if (closestID >= 0) return colors[closestID].r;
        return 0.0f;
    }

    float sampleHeightFromParcelCenter(zVector pt, zVector parcelCenter, float maxDist = 150.0f) // sample the height of the points based on the distance to the parcle center
    {
        float dist = pt.distanceTo(parcelCenter);
        return ofClamp(1.0f - (dist / maxDist), 0.0f, 1.0f);  // closer = taller
    }

    void instanceMeshes(Parcel& parcel)
    {
        for (int i = 0; i < parcel.interiorPoints.size(); i++)
        {
            LandUseType ltype = parcel.interiorPointLandUses[i];
            zVector pos = parcel.interiorPoints[i];

            if (baseMeshes.find(ltype) == baseMeshes.end()) continue; // skip if no mesh loaded

            zObjMesh* instance = new zObjMesh();
            zFnMesh fn(*instance);

            zPointArray verts;
            zIntArray polyConnects, polyCounts;

            zFnMesh baseFn(baseMeshes[ltype]);
            baseFn.getVertexPositions(verts);
            baseFn.getPolygonData(polyConnects, polyCounts);

            // 1. Get direction vector from X-axis (1, 0, 0) to parcel.direction
            zVector from = zVector(1, 0, 0);
            zVector to = parcel.direction;
            from.normalize();
            to.normalize();

            zVector axis = from ^ to; // cross product
            float angle = acos(from * to); // dot product

            zVector toEdge = parcel.getVectorToClosestBoundaryPoint(pos);
            zVector blended = zLerp(parcel.direction, toEdge, weightBlend);
            blended.normalize();

            zTransform rotMatrix = createRotationMatrixFromTo(zVector(1, 0, 0), blended);


            // 2. Rotate mesh vertices
            for (auto& v : verts)
                v = v * rotMatrix;


            // 3. Translate points
            for (auto& v : verts) v += pos; // translate
            fn.create(verts, polyCounts, polyConnects);
            modelPtr->addObject(*instance);

        }
    }

    //void extrudeByVertexColor(zObjMesh& inMesh, float minHeight, float maxHeight)
    //{
    //    zFnMesh fn(inMesh);

    //    zColorArray colors;
    //    fn.getVertexColors(colors);

    //    zFloatArray scalarHeights;
    //    for (auto& c : colors)
    //    {
    //        // Assuming red to black gradient: red = high, black = low
    //        float intensity = c.r; // red component
    //        float height = minHeight + intensity * (maxHeight - minHeight);
    //        scalarHeights.push_back(height);
    //    }

    //    zObjMesh extrudedMesh;
    //    fn.extrudeVariableMesh(scalarHeights, extrudedMesh, true, false); // bothSides=true
    //    fn = zFnMesh(extrudedMesh);
    //    fn.computeMeshNormals();

    //    modelPtr->addObject(extrudedMesh); // add to scene
    //}

    void extrudeParcelMeshes(Parcel& parcel, float minHeight, float maxHeight)
    {
        for (int i = 0; i < parcel.interiorPoints.size(); i++)
        {
            LandUseType ltype = parcel.interiorPointLandUses[i];
            zVector pos = parcel.interiorPoints[i];

            if (baseMeshes.find(ltype) == baseMeshes.end()) continue;

            zObjMesh* tempMesh = new zObjMesh();
            zFnMesh baseFn(baseMeshes[ltype]);
            zFnMesh fn(*tempMesh);

            zPointArray verts;
            zIntArray polyConnects, polyCounts;

            baseFn.getVertexPositions(verts);
            baseFn.getPolygonData(polyConnects, polyCounts);

            zVector toEdge = parcel.getVectorToClosestBoundaryPoint(pos);
            zVector blended = zLerp(parcel.direction, toEdge, weightBlend);
            blended.normalize();

            zTransform rotMatrix = createRotationMatrixFromTo(zVector(1, 0, 0), blended);

            for (auto& v : verts) v = v * rotMatrix;
            for (auto& v : verts) v += pos;

            fn.create(verts, polyCounts, polyConnects);

            //float heightVal = sampleHeightAtPoint(pos);
            float heightVal = sampleHeightFromParcelCenter(pos, parcel.center);

            float extrudeVal = minHeight + heightVal * (maxHeight - minHeight);

            zObjMesh* extrudedMesh = new zObjMesh();
            zFloatArray heights(fn.numVertices(), extrudeVal);

            fn.extrudeVariableMesh(heights, *extrudedMesh, false, false);

            zFnMesh finalFn(*extrudedMesh);
            finalFn.computeMeshNormals();
            modelPtr->addObject(*extrudedMesh);
        }
    }
};


Parcel myParcel;
ParcelMeshManager myParcelMeshManager;


void setup()
{
    model = zModel(100000);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);

    S = *new SliderGroup();
    S.addSlider(&background, "background");
    S.addSlider(&gNumPoints, "Number of Points");
    S.addSlider(&gMinDistance, "Min Distance");
    S.addSlider(&heightMin, "Min Height");  // NEW
    S.addSlider(&heightMax, "Max Height");  // NEW
    S.addSlider(&weightBlend, "Blend Weight");


    S.sliders[0].attachToVariable(&background, 0, 1);
    S.sliders[1].attachToVariable(&gNumPoints, 1, 20);
    S.sliders[2].attachToVariable(&gMinDistance, 0, 100);
    S.sliders[3].attachToVariable(&heightMin, 0, 100);
    S.sliders[4].attachToVariable(&heightMax, 1, 100);
    S.sliders[5].attachToVariable(&weightBlend, 0.0, 1.0);


    B = *new ButtonGroup(vec(50, 450, 0));
    B.addButton(&compute, "compute points");
    B.addButton(&display, "display parcel");
    B.addButton(&instanceMeshes, "instance meshes");
    B.addButton(&extrudeMesh, "extrude mesh");          // NEW
    B.addButton(&showColorHeightMesh, "show colorMesh"); // NEW

    B.buttons[0].attachToVariable(&compute);
    B.buttons[1].attachToVariable(&display);
    B.buttons[2].attachToVariable(&instanceMeshes);
    B.buttons[3].attachToVariable(&extrudeMesh);
    B.buttons[4].attachToVariable(&showColorHeightMesh);

    myParcel.createParcel(zVector(0, 0, 0), 150.0, zVector(1, 1, 0), RESIDENTIAL, { COMMERCIAL, PARK });
    std::vector<float> params = { 0.0, 0.2, 0.35, 0.55, 0.75, 0.9 };
    myParcel.createPolygonFromCircleParams(params);

    myParcelMeshManager.setup(&model);

    myParcelMeshManager.loadMesh(RESIDENTIAL, "C:/Users/Navee.Periyasamy/Documents/Alice_mesh/MVC_1/resi.obj");
    myParcelMeshManager.loadMesh(COMMERCIAL, "C:/Users/Navee.Periyasamy/Documents/Alice_mesh/MVC_1/retail.obj");
    myParcelMeshManager.loadMesh(INDUSTRIAL, "C:/Users/Navee.Periyasamy/Documents/Alice_mesh/MVC_1/industry.obj");
    myParcelMeshManager.loadMesh(PARK, "C:/Users/Navee.Periyasamy/Documents/Alice_mesh/MVC_1/park.obj");

    myParcelMeshManager.loadHeightField("C:/Users/Navee.Periyasamy/Documents/Alice_mesh/MVC_1/ColorMesh1.json");
}

void update(int value)
{
    myParcel.numPoints = std::max(1, (int)gNumPoints);
    myParcel.minDistance = gMinDistance;

    if (compute)
    {
        myParcel.generateInteriorPoints();
        compute = false;
    }

    if (instanceMeshes)
    {
        myParcelMeshManager.instanceMeshes(myParcel);
        instanceMeshes = false;
    }

    if (extrudeMesh)
    {
        myParcelMeshManager.extrudeParcelMeshes(myParcel, heightMin, heightMax);
        extrudeMesh = false;
    }

    if (showColorHeightMesh)
    {
        myParcelMeshManager.loadHeightField("C:/Users/Navee.Periyasamy/Documents/Alice_mesh/MVC_1/ColorMesh1.json");
        model.addObject(*myParcelMeshManager.heightFieldObj);  // ✅ always fresh
        showColorHeightMesh = false;
    }

    if (fabs(weightBlend - prevWeightBlend) > 0.001)
    {
        // Regenerate instance meshes with new blend direction
        //model.removeAll(); // optional: clear old
        myParcelMeshManager.instanceMeshes(myParcel);
        prevWeightBlend = weightBlend;
    }

}


void draw()
{
    backGround(background);
    drawGrid(50);

    S.draw();
    B.draw();

    if (display)
    {
        model.draw();
        myParcel.displayParcel(); // <--- draw the new parcel
    }
}


void keyPress(unsigned char k, int xm, int ym)
{
    if (k == 'g')
    {
        myParcel.numPoints = (int)gNumPoints; // cast!
        myParcel.minDistance = gMinDistance;
        myParcel.generateInteriorPoints();
    }
}

void mousePress(int b, int s, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}



#endif // _MAIN_
