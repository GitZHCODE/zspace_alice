//#define _MAIN_
#ifdef _MAIN_

#include "main.h"
#include <zApp/include/zObjects.h>
#include <zApp/include/zFnSets.h>
#include <zApp/include/zViewer.h>

using namespace zSpace;

//--------------------------------------------
// Utilities
//--------------------------------------------
Alice::vec zVecToAliceVec(const zVector& in) { return Alice::vec(in.x, in.y, in.z); }
zVector AliceVecToZvec(const Alice::vec& in) { return zVector(in.x, in.y, in.z); }

bool isInsideParcel(zVector* polygon, int N, const zVector& p)
{
    int __count = 0;
    zVector p1, p2;
    p1 = polygon[0];
    for (int i = 1; i <= N; ++i)
    {
        if (p.distanceTo(p1) < 1e-6) return false;
        p2 = polygon[i % N];
        if (p.y < MIN(p1.y, p2.y) || p.y > MAX(p1.y, p2.y)) { p1 = p2; continue; }
        if (p.y > MIN(p1.y, p2.y) && p.y < MAX(p1.y, p2.y)) {
            if (p.x <= MAX(p1.x, p2.x)) {
                if (p1.y == p2.y && p.x >= MIN(p1.x, p2.x)) return false;
                if (p1.x == p2.x) {
                    if (p1.x == p.x) return false;
                    else ++__count;
                }
                else {
                    double xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
                    if (fabs(p.x - xinters) < 1e-6) return false;
                    if (p.x < xinters) ++__count;
                }
            }
        }
        p1 = p2;
    }
    return __count % 2 != 0;
}

//--------------------------------------------
// Global Variables
//--------------------------------------------
#define NUM_POINTS 100
zVector packedPoints[NUM_POINTS];
zVector forces[NUM_POINTS];

zObjMesh baseMesh;
zPointArray originalVertices;

zModel model;

class parcel {
public:
    zVector center, direction;
    zTransform TM;
    zVector parcelPoints[50];
    zObjMesh combinedMesh;
    zPointArray positions;
    zIntArray pCounts, pConnects;

    void createShape(float r = 10.0) {
        float inc = TWO_PI / 50.0;
        for (int i = 0; i < 50; i++) {
            float x = r * sin(inc * i);
            float y = r * cos(inc * i);
            parcelPoints[i] = zVector(x, y, 0);
        }
    }

    void transformShape(zVector _c, zVector _dir) {
        direction = _dir;
        TM.setIdentity();
        TM(0, 0) *= 1.5;
        for (int i = 0; i < 50; i++) parcelPoints[i] = parcelPoints[i] * TM;

        zVector u = _dir, v = _dir, w(0, 0, 1);
        v = zVector(-u.y, u.x, 0);
        center = _c;
        u.normalize(); v.normalize();

        TM.col(0) << u.x, u.y, u.z, 1;
        TM.col(1) << v.x, v.y, v.z, 1;
        TM.col(2) << w.x, w.y, w.z, 1;
        TM.col(3) << center.x, center.y, center.z, 1;

        for (int i = 0; i < 50; i++) parcelPoints[i] = parcelPoints[i] * TM;
    }

    void instanceMeshAtPoints(zObjMesh& meshTemplate, const zPointArray& templateVertices, const std::vector<zVector>& centers) {
        positions.clear(); pCounts.clear(); pConnects.clear();
        for (const auto& pt : centers) {
            if (!isInsideParcel(parcelPoints, 50, pt)) continue;

            zTransform instanceTM = TM;
            instanceTM.col(3) << pt.x, pt.y, pt.z, 1;

            zPointArray newVerts = templateVertices;
            for (auto& v : newVerts) v = v * instanceTM;

            zFnMesh fn(baseMesh);
            zIntArray faceCounts, faceConnects;
            fn.getPolygonData(faceConnects, faceCounts);

            int offset = positions.size();
            for (auto& v : newVerts) positions.push_back(v);
            for (auto f : faceCounts) pCounts.push_back(f);
            for (auto f : faceConnects) pConnects.push_back(f + offset);
        }

        zFnMesh fnCombined(combinedMesh);
        fnCombined.create(positions, pCounts, pConnects);
        model.addObject(combinedMesh);
    }
};

parcel myParcel;

//--------------------------------------------
// Core App Functions
//--------------------------------------------
void setup() {
    zFnMesh fn(baseMesh);
    fn.from("data/cube.obj", zOBJ);
    fn.getVertexPositions(originalVertices);

    myParcel.createShape();
    myParcel.transformShape(zVector(0, 0, 0), zVector(1, 1, 0));

    for (int i = 0; i < NUM_POINTS; i++)
        packedPoints[i] = zVector(ofRandom(-5, 5), ofRandom(-5, 5), 0);
}

void update(int value) {
    for (int i = 0; i < NUM_POINTS; i++) forces[i] = zVector();
    for (int i = 0; i < NUM_POINTS; i++) {
        for (int j = 0; j < NUM_POINTS; j++) {
            if (i == j) continue;
            zVector dir = packedPoints[i] - packedPoints[j];
            float dist = dir.length();
            if (dist > 1e-2) {
                dir.normalize();
                forces[i] += dir / (dist * dist);
            }
        }
    }
    for (int i = 0; i < NUM_POINTS; i++)
        if (isInsideParcel(myParcel.parcelPoints, 50, packedPoints[i]))
            packedPoints[i] += forces[i] * 0.05;
}

void draw() {
    backGround(0.8);
    drawGrid(50);

    glColor3f(0, 0, 0);
    for (int i = 0; i < 50; i++)
        drawLine(zVecToAliceVec(myParcel.parcelPoints[i]), zVecToAliceVec(myParcel.parcelPoints[(i + 1) % 50]));

    glPointSize(5);
    glColor3f(1, 0, 0);
    for (int i = 0; i < NUM_POINTS; i++)
        drawPoint(zVecToAliceVec(packedPoints[i]));

    model.draw();
}

void keyPress(unsigned char k, int xm, int ym) {
    if (k == 'm') {
        std::vector<zVector> centers(packedPoints, packedPoints + NUM_POINTS);
        myParcel.instanceMeshAtPoints(baseMesh, originalVertices, centers);
    }
}

void mousePress(int b, int state, int x, int y) {}
void mouseMotion(int x, int y) {}

#endif // _MAIN_
