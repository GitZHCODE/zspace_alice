#define _MAIN_
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

#define RES 50

class Employment_center
{
public:
    zVector cen;
    zVector clr;
    zVector target;

    void drawTheBox()
    {
        zVector minPt(-1, -0.25, 0);
        zVector maxPt(1, 0.25, 0);

        minPt = minPt + cen;
        maxPt = maxPt + cen;

        glColor3f(clr.x, clr.y, clr.z);
        drawRectangle(zVecToAliceVec(minPt), zVecToAliceVec(maxPt));
    }
};

struct Node {
    int x, y;
    float cost;
    float heuristic;
    Node* parent;

    Node(int x, int y, float cost, float heuristic, Node* parent)
        : x(x), y(y), cost(cost), heuristic(heuristic), parent(parent) {}
};

struct CompareNode {
    bool operator()(Node* a, Node* b) {
        return (a->cost + a->heuristic) > (b->cost + b->heuristic);
    }
};

class landValueMap
{
public:
    float lv[RES][RES];
    zVector lvec[RES][RES];
    std::vector<zVector> lastShortestPath;

    void updateLVMap(const std::vector<Employment_center>& ECenters)
    {
        for (int i = 0; i < RES; i++)
        {
            for (int j = 0; j < RES; j++)
            {
                zVector pt(i, j, 0);
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
                lv[i][j] = (totalWeight > 0.0f) ? weightedSum / totalWeight : 0.0f;
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

        while (!openSet.empty()) {
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

                if (nx >= 0 && nx < RES && ny >= 0 && ny < RES) {
                    float newCost = current->cost + lv[nx][ny] + 1;
                    openSet.push(new Node(nx, ny, newCost, heuristic(nx, ny, ex, ey), current));
                }
            }
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

        glColor3f(0, 0, 0);
        for (int i = 1; i < RES ; i++)
        {
            for (int j = 1; j < RES ; j++)
            {

                zVector pt, dir;
                pt = zVector(i, j, 0);
                dir = lvec[i][j];
                dir.normalize();

                drawLine(zVecToAliceVec(pt), zVecToAliceVec(pt+dir));
                

               // drawPt()
            }
        }
    }
};

std::vector<Employment_center> listofEmploymentCenters;
landValueMap myLVM;


void setup() {}

void update(int value) {}

void draw()
{
    backGround(0.8);
    drawGrid(50);
    for (auto& center : listofEmploymentCenters) {
        center.drawTheBox();
    }
    myLVM.draw();
}

void keyPress(unsigned char k, int xm, int ym)
{
    if (k == '+')
    {
        Employment_center newCenter;
        newCenter.cen = zVector(ofRandom(0, 50), ofRandom(0, 50), 0);
        listofEmploymentCenters.push_back(newCenter);
        myLVM.updateLVMap(listofEmploymentCenters);
    }
    if (k == 'v')
    {
        myLVM.computeVectorField();
    }
    if (k == 'p')
    {
        zVector str, end;
        str = listofEmploymentCenters[0].cen;//zVector(ofRandom(0, 50), ofRandom(0, 50), 0)
        end = listofEmploymentCenters[1].cen;//zVector(ofRandom(0, 50), ofRandom(0, 50), 0)

        myLVM.findShortestPath(str,end);
    }
}

void mousePress(int b, int state, int x, int y) {}

void mouseMotion(int x, int y) {}

#endif // _MAIN_
