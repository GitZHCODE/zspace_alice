#define _MAIN_
#ifdef _MAIN_

#include "main.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>


using namespace zSpace;

Alice::vec zVecToAliceVec(zVector& in)
{
    return Alice::vec(in.x, in.y, in.z);
}

#include "scalarField.h"

#pragma once

#include "scalarField.h"
#include <fstream>
#include <sstream>
#include <map>

class HeightField2D : public ScalarField2D
{
public:

    std::vector<zVector> samples;
    double zMin = 0.0f, zMax = 1.0f;
    double zScale = 5;

    void readSamplesAndInterpolate(const std::string& filename)
    {
        samples.clear();

        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Failed to open " << filename << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string xStr, yStr, zStr;

            if (std::getline(ss, xStr, ',') &&
                std::getline(ss, yStr, ',') &&
                std::getline(ss, zStr))
            {
                float x = std::stof(xStr);
                float y = std::stof(yStr);
                float z = std::stof(zStr);
                samples.emplace_back(x, y, z);
            }
        }

        file.close();

        rescaleSamplesToBoundingBox(zVector(-50, -50, -50), zVector(50, 50, 50));

        interpolateToGrid();
    }

    void rescaleSamplesToBoundingBox( zVector& targetMin,  zVector& targetMax)
    {
        if (samples.empty()) return;

        zVector bmin = samples[0];
        zVector bmax = samples[0];

        for (const auto& s : samples)
        {
            bmin.x = std::min(bmin.x, s.x);
            bmin.y = std::min(bmin.y, s.y);
            bmax.x = std::max(bmax.x, s.x);
            bmax.y = std::max(bmax.y, s.y);
        }

        zVector scale = targetMax - targetMin;
        zVector dataRange = bmax - bmin;

        for (auto& s : samples)
        {
            s.x = targetMin.x + (s.x - bmin.x) / dataRange.x * scale.x;
            s.y = targetMin.y + (s.y - bmin.y) / dataRange.y * scale.y;
            // z remains untouched for now
        }

        // compute zMin/zMax for later use
        zMin = samples[0].z;
        zMax = samples[0].z;
        for (auto& s : samples)
        {
            zMin = std::min(float(zMin), s.z);
            zMax = std::max(float(zMax), s.z);
        }
    }


    void interpolateToGrid()
    {
        for (int i = 0; i < RES; i++)
        {
            for (int j = 0; j < RES; j++)
            {
                zVector gp = gridPoints[i][j];
                float num = 0.0f;
                float den = 0.0f;

                for (const auto& s : samples)
                {
                    float d = gp.distanceTo(zVector(s.x, s.y, 0));
                    if (d < 1e-3f) d = 1e-3f;

                    float w = 1.0f / (d * d);
                    num += w * s.z;
                    den += w;
                }

                field[i][j] = (den > 0.0f) ? num / den : 0.0f;
            }
        }

        normalise();
    }

    void setGridPointHeights()
    {
        if (samples.empty()) return;

        for (int i = 0; i < RES; i++)
        {
            for (int j = 0; j < RES; j++)
            {
               gridPoints[i][j].z = ofMap(field_normalized[i][j], 0, 1, -zScale, zScale);
            }
        }
    }


    void drawSamplePoints()
    {
        if (samples.empty()) return;

        glPointSize(5);
        glBegin(GL_POINTS);
        for (const auto& ptRaw : samples)
        {
            zVector pt = ptRaw;
            pt.z = ofMap(ptRaw.z, zMin, zMax, -1.0f, 1.0f) * zScale; // normalized and scaled

            float color = ofMap(pt.z, -zScale, zScale, 0.0f, 1.0f);
            glColor3f(color, 0.0f, 1.0f - color);

            Alice::vec av = zVecToAliceVec(pt);
            glVertex3f(av.x, av.y, av.z);
        }
        glEnd();
        glPointSize(1);
    }

};


HeightField2D myHeightField;
double threshold;

void setup()
{
 
    S.addSlider(&threshold, "tv");// make a slider control for the variable called width;
    S.sliders[0].maxVal = 1;

    myHeightField.readSamplesAndInterpolate("data/roatan.txt");
    myHeightField.setGridPointHeights();
}

void update(int value)
{
}

void draw()
{
    backGround(0.9);
    drawGrid(50);

 
   // myHeightField.drawSamplePoints();
    myHeightField.drawFieldPoints();
    
    glLineWidth(5);
    for( double tv = 0 ; tv < threshold ; tv += 0.05) myHeightField.drawIsocontours(tv);
    glLineWidth(1);


}

void keyPress(unsigned char k, int xm, int ym)
{
     

   
}

void mousePress(int b, int state, int x, int y)
{
}

void mouseMotion(int x, int y)
{
}

#endif // _MAIN_
