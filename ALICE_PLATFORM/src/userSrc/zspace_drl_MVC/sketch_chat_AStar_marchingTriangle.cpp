//#define _MAIN_
#ifdef _MAIN_

#include "main.h"
#include <queue>
#include <unordered_map>


//#include "alice/spatialBin.h"
//// zSpace Core Headers
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

using namespace zSpace;

Alice::vec zVecToAliceVec(zVector& in)
{
    return Alice::vec(in.x, in.y, in.z);
}

zVector AliceVecToZvec(Alice::vec& in)
{
    return zVector(in.x, in.y, in.z);
}

#define RES 100

class ProgramCenter
{
public:

    enum Type { RESI, OFFICE, RETAIL };  // Rename enum to 'Type'
    Type type; // Declare a variable of the enum type

    zVector cen;
    zVector clr;
    zVector target;

    void draw()
    {
        clr = (type == OFFICE) ? zVector(1, 0, 0) : zVector(0, 0, 0);

        zVector minPt(-0.25, -0.25, 0);
        zVector maxPt( 0.25, 0.25, 0);

        minPt = minPt + cen;
        maxPt = maxPt + cen;

        glColor3f(clr.x, clr.y, clr.z);
        drawRectangle(zVecToAliceVec(minPt), zVecToAliceVec(maxPt));
    }
};



struct Node 
{
    int x, y;
    float cost;
    float heuristic;
    Node* parent;

    Node(int x, int y, float cost, float heuristic, Node* parent)
        : x(x), y(y), cost(cost), heuristic(heuristic), parent(parent) {}
};

struct CompareNode 
{
    bool operator()(Node* a, Node* b) 
    {
        return (a->cost + a->heuristic) > (b->cost + b->heuristic);
    }
};

class landValueMap
{
public:
    
    zVector lpt[RES][RES];
    float lv[RES][RES];
    float lv_norm[RES][RES];
    zVector lvec[RES][RES];
    std::vector<zVector> lastShortestPath;
    std::vector< std::pair<zVector, zVector >> isolines;

    void updateLVMap(const std::vector<ProgramCenter>& ECenters)
    {
        for (int i = -(RES * 0.5), icnt = 0; i < (RES * 0.5); i++ , icnt++)
        {
            for (int j = -(RES * 0.5), jcnt = 0; j < (RES * 0.5); j++ , jcnt++)
            {
                lpt[icnt][jcnt] = zVector (i, j, 0);
                zVector pt = lpt[icnt][jcnt];

                float weightedSum = 0.0f;
                float totalWeight = 0.0f;
                for (const auto& EC : ECenters)
                {
                    zVector cen = EC.cen;
                    float distance = pt.distanceTo(cen);
                    float weight = (distance > 0.0f) ? 1.0f / distance : 1.0f;
                    weightedSum += weight * distance;
                    totalWeight += weight;
                }
                lv[icnt][jcnt] = (totalWeight > 0.0f) ? weightedSum / totalWeight : 0.0f;
            }
        }
    }

    void normaliseField()
    {
        float mx, mn;
        mx = -1e6; mn = 1e6;

        for (int i = 0; i < RES ; i++)
        {
            for (int j = 0; j < RES ; j++)
            {
                mx = max(mx, lv[i][j]);
                mn = min(mn, lv[i][j]);
            }
        }

        for (int i = 0; i < RES; i++)
        {
            for (int j = 0; j < RES; j++)
            {
                lv_norm[i][j] = ofMap(lv[i][j], mn, mx, 0, 1);
            }
        }
    }
    void computeVectorField()
    {
        for (int i = 1; i < RES - 1; i++)
        {
            for (int j = 1; j < RES - 1; j++)
            {
                float dx = (lv[i + 1][j] - lv[i - 1][j]) * 0.5f;
                float dy = (lv[i][j + 1] - lv[i][j - 1]) * 0.5f;
                lvec[i][j] = zVector(dx, dy, 0);
            }
        }
    }

    void findShortestPath(zVector start, zVector end)
    {
        lastShortestPath.clear();

        auto heuristic = [](int x1, int y1, int x2, int y2) {
            return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
            };

        int sx = std::round(start.x), sy = std::round(start.y);
        int ex = std::round(end.x), ey = std::round(end.y);

        std::priority_queue<Node*, std::vector<Node*>, CompareNode> openSet;
        std::unordered_map<int, Node*> visited;

        Node* startNode = new Node(sx, sy, 0, heuristic(sx, sy, ex, ey), nullptr);
        openSet.push(startNode);

        int directions[4][2] = { {0,1}, {1,0}, {0,-1}, {-1,0} };

        while (!openSet.empty()) 
        {
            Node* current = openSet.top();
            openSet.pop();

            if (current->x == ex && current->y == ey) {
                while (current) {
                    lastShortestPath.push_back(zVector(current->x, current->y, 0));
                    current = current->parent;
                }
                std::reverse(lastShortestPath.begin(), lastShortestPath.end());
                return;
            }

            int key = current->x * RES + current->y;
            if (visited.count(key)) continue;
            visited[key] = current;

            for (auto& dir : directions) {
                int nx = current->x + dir[0];
                int ny = current->y + dir[1];

                if (nx >= 0 && nx < RES && ny >= 0 && ny < RES) 
                {
                    float newCost = current->cost + (lv[nx][ny]) + 1;
                    openSet.push(new Node(nx, ny, newCost, heuristic(nx, ny, ex, ey), current));
                }
            }
        }


    }

    void findShortestPathDijkstra(zVector start, zVector end, bool invert = false)
    {
        lastShortestPath.clear();

        int offset = RES / 2;  // Shift negative coordinates
        int sx = std::round(start.x) + offset, sy = std::round(start.y) + offset;
        int ex = std::round(end.x) + offset, ey = std::round(end.y) + offset;

        auto compare = [](Node* a, Node* b) { return a->cost > b->cost; };
        std::priority_queue<Node*, std::vector<Node*>, decltype(compare)> openSet(compare);
        std::unordered_map<int, Node*> visited;

        Node* startNode = new Node(sx, sy, 0, 0, nullptr);
        openSet.push(startNode);

        int directions[6][2] = { {0,1}, {1,0}, {0,-1}, {-1,0},{-1,-1},{1,1} };

        while (!openSet.empty())
        {
            Node* current = openSet.top();
            openSet.pop();

            if (current->x == ex && current->y == ey) {
                while (current) {
                    lastShortestPath.push_back(lpt[current->x][current->y]); // Use lpt[][] instead
                    current = current->parent;
                }
                std::reverse(lastShortestPath.begin(), lastShortestPath.end());
                return;
            }

            int key = (current->x) * RES + (current->y);  // Now key is always positive
            if (visited.count(key)) continue;
            visited[key] = current;

            for (auto& dir : directions) {
                int nx = current->x + dir[0];
                int ny = current->y + dir[1];

                if (nx >= 0 && nx < RES && ny >= 0 && ny < RES)
                {
                    float newCost = current->cost + (invert ? (1.0 / lv[nx][ny]) : lv[nx][ny]) + 1;
                    openSet.push(new Node(nx, ny, newCost, 0, current));
                }
            }
        }
    }


    void smoothPath( vector<zVector> &path)
    {
        if (path.size() < 3) return; // No smoothing if too few points

        std::vector<zVector> smoothedPath = path;

        for (size_t i = 1; i < path.size() - 1; i++)
        {
            smoothedPath[i] = path[i - 1] * 0.3f + path[i] * 0.4f + path[i + 1] * 0.3f;
        }

        path = smoothedPath;
    }
    
    void smoothPath()
    {
        if (lastShortestPath.size() < 3) return; // No smoothing if too few points

        std::vector<zVector> smoothedPath = lastShortestPath;

        for (size_t i = 1; i < lastShortestPath.size() - 1; i++)
        {
            smoothedPath[i] = lastShortestPath[i - 1] * 0.3f + lastShortestPath[i] * 0.4f + lastShortestPath[i + 1] * 0.3f;
        }

        lastShortestPath = smoothedPath;
    }

    void drawPth(vector<zVector>& path)
    {
        if (!path.empty())
        {
            glColor3f(0, 0, 1);
            for (size_t i = 0; i < path.size() - 1; i++)
            {
                drawLine(zVecToAliceVec(path[i]), zVecToAliceVec(path[i + 1]));
            }
        }
    }

    void processTriangle(zVector pts[3], float values[3], float thresholdValue, std::vector<std::pair<zVector, zVector>>& contour)
    {
        std::vector<zVector> edgePoints;

        for (int i = 0; i < 3; i++)
        {
            int next = (i + 1) % 3;
            if ((values[i] < thresholdValue && values[next] >= thresholdValue) ||
                (values[next] < thresholdValue && values[i] >= thresholdValue))
            {
                float t = (thresholdValue - values[i]) / (values[next] - values[i]);
                edgePoints.push_back(pts[i] + (pts[next] - pts[i]) * t);
            }
        }

        if (edgePoints.size() == 2)
        {
            contour.push_back({ edgePoints[0], edgePoints[1] });
        }
    }

    void processAllTriangles(float thresholdValue, std::vector<std::pair<zVector, zVector>>& contours)
    {
        normaliseField();
        int offset = RES / 2; // Shift negative coordinates

        for (int i = -offset; i < offset - 1; i++)
        {
            for (int j = -offset; j < offset - 1; j++)
            {
                int iIdx = i + offset;
                int jIdx = j + offset;
                int i1Idx = iIdx + 1;
                int j1Idx = jIdx + 1;

                zVector quadPts[4] = { lpt[iIdx][jIdx], lpt[iIdx][j1Idx], lpt[i1Idx][jIdx], lpt[i1Idx][j1Idx] };
                float quadValues[4] = { lv_norm[iIdx][jIdx], lv_norm[iIdx][j1Idx], lv_norm[i1Idx][jIdx], lv_norm[i1Idx][j1Idx] };

                zVector tri1Pts[3] = { quadPts[0], quadPts[1], quadPts[2] };
                float tri1Values[3] = { quadValues[0], quadValues[1], quadValues[2] };
                processTriangle(tri1Pts, tri1Values, thresholdValue, contours);

                zVector tri2Pts[3] = { quadPts[1], quadPts[2], quadPts[3] };
                float tri2Values[3] = { quadValues[1], quadValues[2], quadValues[3] };
                processTriangle(tri2Pts, tri2Values, thresholdValue, contours);
            }
        }
    }

    void rotateVectorFieldAndComputeScalar()
    {
        computeVectorField();
        
        zVector rotatedVec[RES][RES];
        float newScalarField[RES][RES] = { 0 };
        float minVal = 1e6, maxVal = -1e6;
        float stepLimit = 5.0f;  // Prevents massive numerical jumps
        int maxIterations = 100; // Convergence control

        // Step 1: Rotate the vector field by 90 degrees (counterclockwise)
        for (int i = 0; i < RES; i++)
        {
            for (int j = 0; j < RES; j++)
            {
                rotatedVec[i][j] = zVector(-lvec[i][j].y, lvec[i][j].x, 0);
            }
        }

        // Step 2: Iterative integration to compute the scalar field
        for (int iter = 0; iter < maxIterations; iter++) // Converging over iterations
        {
            for (int i = 1; i < RES - 1; i++)
            {
                for (int j = 1; j < RES - 1; j++)
                {
                    float dx = (rotatedVec[i + 1][j].x - rotatedVec[i - 1][j].x) * 0.5f;
                    float dy = (rotatedVec[i][j + 1].y - rotatedVec[i][j - 1].y) * 0.5f;

                    // Clamp step sizes to avoid instability
                    dx = std::max(-stepLimit, std::min(stepLimit, dx));
                    dy = std::max(-stepLimit, std::min(stepLimit, dy));

                    // Jacobi-like update (smooth & stable)
                    newScalarField[i][j] = 0.25f * (newScalarField[i - 1][j] + newScalarField[i + 1][j] +
                        newScalarField[i][j - 1] + newScalarField[i][j + 1] + dx + dy);

                    // Prevent invalid values
                    if (!std::isfinite(newScalarField[i][j]))
                        newScalarField[i][j] = 0.0f;

                    // Track min/max values
                    minVal = std::min(minVal, newScalarField[i][j]);
                    maxVal = std::max(maxVal, newScalarField[i][j]);
                }
            }
        }

        // Step 3: Normalize the field into a positive range
        float range = maxVal - minVal;
        if (range == 0) range = 1;  // Avoid division by zero

        for (int i = 0; i < RES; i++)
        {
            for (int j = 0; j < RES; j++)
            {
                newScalarField[i][j] = (newScalarField[i][j] - minVal) / range;
                lvec[i][j] = rotatedVec[i][j];  // Store rotated vectors
                lv[i][j] = newScalarField[i][j];
            }
        }

        // Debug: Print Min and Max values
        printf("Normalized Scalar Field - Min: %.6f, Max: %.6f\n", minVal, maxVal);
    }

    void drawStreamlinesFromProgramCenters
    (
        vector<ProgramCenter>& listofProgramCenters,
        int numDirections = 16, float stepSize = 0.5f, int maxSteps = 400
    )
    {
        std::vector<std::vector<zVector>> streamlines;

        int offset = RES / 2;  // Shift negative coordinates

        // Step 1: Iterate through each Employment Center as a seed
        for (const auto& center : listofProgramCenters)
        {
            zVector seed = center.cen;

            // Step 2: Generate streamlines in radial directions
            for (int d = 0; d < numDirections; d++)
            {
                float angle = (float)d / numDirections * TWO_PI;  // Convert to radians
                zVector direction(cos(angle), sin(angle), 0); // Initial direction

                zVector p = seed;
                p += direction;
                std::vector<zVector> path;
                path.push_back(p);

                // Step 3: Integrate streamline using RK2
                for (int i = 0; i < maxSteps; i++)
                {
                    // 🔹 **Ensure correct coordinate indexing**
                    int ix = std::round(p.x) + offset;
                    int iy = std::round(p.y) + offset;
                    ix = std::clamp(ix, 0, RES - 1);
                    iy = std::clamp(iy, 0, RES - 1);

                    // 🔹 **Correct nearest vector sampling**
                    zVector v1 = lvec[ix][iy];

                    if (v1.length() < 1e-6)
                        v1 = direction; // Use initial radial direction if no valid vector

                    v1.normalize();
                    v1 *= stepSize;

                    // RK2 Midpoint Method
                    zVector midPoint = p + v1 * 0.5f;
                    int mx = std::round(midPoint.x) + offset;
                    int my = std::round(midPoint.y) + offset;
                    mx = std::clamp(mx, 0, RES - 1);
                    my = std::clamp(my, 0, RES - 1);
                    zVector v2 = lvec[mx][my];

                    if (v2.length() < 1e-6)
                        v2 = direction;

                    v2.normalize();
                    v2 *= stepSize;

                    // Update position using RK2
                    p = p + v2;

                    // Stop if out of bounds
                    if (p.x < -offset || p.x >= offset || p.y < -offset || p.y >= offset)
                        break;

                    path.push_back(p);
                }

                // Store streamline path
                streamlines.push_back(path);
            }
        }

        // Step 4: Draw streamlines
        glColor3f(0, 1, 0); // Blue streamlines
        glLineWidth(3);
        for ( auto streamline : streamlines)
        {
            for (int n = 0; n < 3; n++)smoothPath(streamline);

            for (size_t i = 0; i < streamline.size() - 1; i++)
            {
                zVector str = streamline[i];
                zVector end = streamline[i + 1];

                drawLine(zVecToAliceVec(str), zVecToAliceVec(end));
            }
        }

        glLineWidth(1);
    }



    void drawIsoline(float thresholdValue)
    {
        isolines.clear();
        processAllTriangles(thresholdValue, isolines);

        glColor3f(1, 1, 0);
        for (const auto& segment : isolines)
        {
            zVector str, end;
            str = segment.first;
            end = segment.second;

            drawLine( zVecToAliceVec(str), zVecToAliceVec(end) );
        }
    }
    void draw()
    {

        

        if (!lastShortestPath.empty())
        {
            glColor3f(0, 0, 1);
            for (size_t i = 0; i < lastShortestPath.size() - 1; i++)
            {
                drawLine(zVecToAliceVec(lastShortestPath[i]), zVecToAliceVec(lastShortestPath[i + 1]));
            }
        }


        float mx = -1e6;
        for (int i = 1; i < RES; i++)
            for (int j = 1; j < RES; j++)
                mx = max(mx, lv[i][j]);
       
        glPointSize(2);

        for (int i = 1; i < RES; i++)
        {
            for (int j = 1; j < RES; j++)
            {

                zVector pt, dir;
                pt = lpt[i][j];//  zVector(i, j, 0);
                dir = lvec[i][j];
                dir.normalize();

                float r = ofMap(lv[i][j], 0, mx, 0, 1);

                glColor3f(r, 0, 0);
                drawPoint(zVecToAliceVec(pt)); 

                glColor3f(0, 0, 0);
                drawLine(zVecToAliceVec(pt), zVecToAliceVec(pt + dir));

            }
        }
    }
};


class Agent
{
public:

    zVector pos, target;
    int cnt;
    zVector origin, dest;

    vector<zVector> path;
    bool rev = false;

    Agent()
    {
        origin = zVector(ofRandom(-50, 50), ofRandom(-50, 50), 0);
        dest = zVector(ofRandom(-50, 50), ofRandom(-50, 50), 0);
    }

    void getPath(landValueMap& lvm)
    {
        lvm.findShortestPathDijkstra(origin, dest);
        path = lvm.lastShortestPath;
        
        if (path.size() >= 2)
        {
            pos = path[0];
            target = path[1];
            cnt = 2;
        }
       
    }

    void update()
    {
        if (pos.distanceTo(target) < 2e-1 && path.size() >= 2)
        {
            target = path[cnt];
            cnt += rev ? -1 : 1;

            if (cnt >= path.size())
            {
                cnt = path.size()-1;
                rev = true;
            }

            if (cnt == 0)
            {
                rev = false;
                cnt = 1;
            }
        }

        zVector dir = (target - pos);
        dir.normalize();
        pos += dir * 0.1;
    }

    void draw()
    {
        glPointSize(5);
        glColor3f(1, 0, 0);

        drawPoint(zVecToAliceVec(pos));

        glLineWidth(5);
        if (path.size() >= 2)
            for (int i = 0; i < path.size() - 1; i++)
                drawLine(zVecToAliceVec(path[i]), zVecToAliceVec(path[i + 1]));
        glLineWidth(1);

        glPointSize(2);
        glColor3f(0, 0, 0);
        if (path.size() >= 2)
            for (int i = 0; i < path.size() - 1; i++)
                drawPoint(zVecToAliceVec(path[i]));

        glPointSize(1);
    }
};

#define nPoly 64






    

/// <summary>
/// ----------------------------------------------------------------------- MVC application
/// </summary>
//global variables
std::vector<ProgramCenter> listofCenters;
landValueMap myLVM;
vector < vector<zVector> > allPaths;

double thresValue = 0.5;
double maxHt = 2.0;

Agent resident;



void setup() 
{

    S.addSlider(&thresValue, "tv");// make a slider control for the variable called width;
    S.sliders[0].maxVal = 1;

    S.addSlider(&maxHt, "maxHt");// make a slider control for the variable called width;
    S.sliders[1].maxVal = 20;

    
    
}

void update(int value) 
{
    resident.update();
}

void draw()
{
    backGround(0.8);
    drawGrid(50);
    for (auto& center : listofCenters) 
    {
        center.draw();
    }
    myLVM.draw();

    for (auto& path : allPaths)
    {
        myLVM.drawPth(path);
    }

    for (float i = 0.0; i < thresValue; i+=0.025)
    {
        myLVM.drawIsoline(i);
    }

    myLVM.drawStreamlinesFromProgramCenters(listofCenters);

    //

    resident.draw();
}

int cnt = 0;
void keyPress(unsigned char k, int xm, int ym)
{
    if (k == '4')
    {
        resident.getPath(myLVM);
        for (int i = 0; i < 15; i++)
        {
            myLVM.smoothPath(resident.path);
        }

    }
    
    if (k == '5')
    {
        resident.update();
    }
    ///
    
    if (k == '+')
    {
        ProgramCenter newCenter;
        newCenter.type = ProgramCenter::OFFICE;
        newCenter.cen = zVector(ofRandom(-(RES * 0.5), RES * 0.5)+5, ofRandom(-(RES * 0.5), RES * 0.5)+5, 0);
        listofCenters.push_back(newCenter);
        myLVM.updateLVMap(listofCenters);
    }
    if (k == 'v')
    {
        myLVM.computeVectorField();
    }
    
    if (k == 'y')
    {
        myLVM.updateLVMap(listofCenters);
    }


    if (k == 't')
    {
        myLVM.rotateVectorFieldAndComputeScalar();
    }


    if (k == '1' || k == '2')
    {
        allPaths.clear();

        zVector str, end;
        for (int i = 1 ; i < listofCenters.size() ; i++)
        {

            str = listofCenters[0].cen;//zVector(ofRandom(0, 50), ofRandom(0, 50), 0)
            end = listofCenters[i].cen;//zVector(ofRandom(0, 50), ofRandom(0, 50), 0)

            myLVM.findShortestPathDijkstra(str, end, (k == '1'));

            for (int j = 0; j < 15; j++)
            {
                myLVM.smoothPath();
            }

            allPaths.push_back(myLVM.lastShortestPath);
        }
        
    }

    if (k == 'r')
    {
        allPaths.clear();

        cnt++;
        if (cnt >= listofCenters.size())cnt = 1;
        zVector str, end;
        str = zVector(ofRandom(-50, 50), ofRandom(-50, 50), 0);
        end = zVector(ofRandom(-50, 50), ofRandom(-50, 50), 0);

        myLVM.findShortestPathDijkstra(str, end,true);
    }

    if (k == 's')myLVM.smoothPath(); 

    if (k == 'h')
    {
        float curMaxHt = -1e6;
        for (int i = 0; i < RES; i++)
            for (int j = 0; j < RES; j++)
                curMaxHt = max(myLVM.lv[i][j], curMaxHt);

        for (int i = 0; i < RES; i++)
            for (int j = 0; j < RES; j++) 
               myLVM.lpt[i][j].z = ofMap(myLVM.lv[i][j],0,curMaxHt,0,maxHt);
    }

    ///

    if (k == '0')
    {
        allPaths.clear();

        cnt++;
        if (cnt >= listofCenters.size())cnt = 1;
        zVector str, end;
        str = listofCenters[0].cen;//zVector(ofRandom(0, 50), ofRandom(0, 50), 0)
        end = zVector(ofRandom(0, 50), ofRandom(0, 50), 0);//listofEmploymentCenters[cnt].cen;

        myLVM.findShortestPathDijkstra(str, end);
    }
}

void mousePress(int b, int state, int x, int y) {}

void mouseMotion(int x, int y) {}

#endif // _MAIN_
