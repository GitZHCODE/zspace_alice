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


#include <userSrc/tzSketches/Landuse/zTsConfiguratorLanduse.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <map>
#include <chrono>
#include <limits>
#include <iomanip>
#include <queue>

using namespace std;
using namespace std::chrono;

namespace zSpace
{
    // zTsConfiguratorLanduse

    // PUBLIC

    zTsConfiguratorLanduse::zTsConfiguratorLanduse() {};
    zTsConfiguratorLanduse::~zTsConfiguratorLanduse() {};

    void zTsConfiguratorLanduse::initialise(string path)
    {
        zFnMesh fnMesh(underlayMesh);
        fnMesh.from(path, zUSD);

        displayMesh = zObjMesh(underlayMesh);

        // set rows and columns
        zItMeshVertex v(underlayMesh, 0);
        zIntArray he_ids;
        v.getConnectedHalfEdges(he_ids);
        zItMeshHalfEdge he_row(underlayMesh, he_ids[0]);
		zItMeshHalfEdge he_col(underlayMesh, he_ids[1]);

        if (he_row.onBoundary()) swap(he_row, he_col);
        he_row = he_row.getSym();

        ROWS = 0;
        COLS = 0;

        zItMeshHalfEdge he_temp_row = he_row;
        zItMeshHalfEdge he_temp_col = he_col;

        do
        {
            COLS++;
            he_temp_row = he_temp_row.getNext();
        } while (!he_temp_row.getVertex().checkValency(2));

        do
        {
            ROWS++;
            he_temp_col = he_temp_col.getPrev();
        } while (!he_temp_col.getStartVertex().checkValency(2));

        cout << "rows:" << ROWS << endl;
        cout << "cols:" << COLS << endl;


        grid = vector<vector<CellType>>(ROWS, vector<CellType>(COLS, EMPTY));
        gridFaceIds = vector<vector<int>>(ROWS, vector<int>(COLS, -1));

        int ct_row = 0;
        do
        {
            he_col = he_row.getSym().getNext();
            int ct_col = 0;

            do
            {
                zColor color = he_col.getFace().getColor();
                gridFaceIds[ct_row][ct_col] = he_col.getFace().getId();

                //if (col == zColor(1, 1, 1)) grid[ROWS][COLS] = EMPTY;
				if      (color == zColor(0, 0, 0, 1)) grid[ct_row][ct_col] = ROAD;
				else if (color == zColor(1, 0, 0, 1)) grid[ct_row][ct_col] = TRANSPORT;
				else if (color == zColor(0, 1, 0, 1)) grid[ct_row][ct_col] = LANDSCAPE;
				else if (color == zColor(1, 0, 1, 1)) grid[ct_row][ct_col] = PUBLIC;

                //cout << "-------" << endl;
                //cout << ct_row << "," << ct_col << endl;
                ////cout << color.r << "," << color.g << "," << color.b << endl;
                //cout << he_row.getStartVertex().getId() << " " << he_row.getVertex().getId() << endl;
                //cout << he_col.getStartVertex().getId() << " " << he_col.getVertex().getId() << endl;

                ct_col++;
                he_col = he_col.getNext().getSym().getNext();
			} while (ct_col < COLS);

			ct_row++;
			he_row = he_row.getPrev();
		} while (ct_row < ROWS);
            
        cout << "Input Grid:" << endl;
        printGrid(grid);

        // Compute distance maps
        auto distanceStart = high_resolution_clock::now();
        distanceMaps = computeDistanceMaps(grid);
        auto distanceEnd = high_resolution_clock::now();
        auto distanceDuration = duration_cast<milliseconds>(distanceEnd - distanceStart);
        cout << "Distance Maps Computation Time: " << distanceDuration.count() << " milliseconds" << endl;

        auto initialScoreStart = high_resolution_clock::now();
        double initialScore = calculateScore(grid, distanceMaps);
        auto initialScoreEnd = high_resolution_clock::now();
        auto initialScoreDuration = duration_cast<milliseconds>(initialScoreEnd - initialScoreStart);
        cout << "Initial Score: " << initialScore << endl;
        cout << "Initial Score Computation Time: " << initialScoreDuration.count() << " milliseconds" << endl;

        generateGrid_input(grid, agentPercentages);
        //generateGrid_heuristic(grid, agentPercentages, distanceMaps);
        cout << "Initial Grid:" << endl;
        printGrid(grid);
    }

    void zTsConfiguratorLanduse::compute(double _temperature,double _cooldown,double _coolingRate)
    {
        auto optimisationStart = high_resolution_clock::now();

        optimiseGrid(grid, distanceMaps, _temperature, _cooldown, _coolingRate);

        auto optimisationEnd = high_resolution_clock::now();
        auto optimisationDuration = duration_cast<milliseconds>(optimisationEnd - optimisationStart);

        cout << "\nOptimised Grid:" << endl;
        printGrid(grid);

        auto finalScoreStart = high_resolution_clock::now();
        finalScore = calculateScore(grid, distanceMaps);
        auto finalScoreEnd = high_resolution_clock::now();
        auto finalScoreDuration = duration_cast<milliseconds>(finalScoreEnd - finalScoreStart);
        cout << "Optimised Score: " << finalScore << endl;
        cout << "Optimised Score Computation Time: " << finalScoreDuration.count() << " milliseconds" << endl;

        cout << "Optimisation Time: " << optimisationDuration.count() << " milliseconds" << endl;
    }

    void zTsConfiguratorLanduse::to(string path)
    {
        zFnMesh fnMesh(displayMesh);
        fnMesh.to(path, zUSD);
    }

    // PRIVATE

    // Function to print the grid
    void zTsConfiguratorLanduse::printGrid(const vector<vector<CellType>>& grid)
    {
        //for (auto row_it = grid.rbegin(); row_it != grid.rend(); ++row_it)
        //{
        //    for (auto& cell = row_it->rbegin(); cell != row_it->rend(); ++cell)
        for (const auto& row : grid)
        {
            //for (auto& cell = row.rbegin(); cell != row.rend(); ++cell)
            for (const auto& cell : row)
            {
                char c;
                switch (cell)
                {
                case RESIDENTIAL:
                    c = 'R';
                    break;
                case OFFICE:
                    c = 'O';
                    break;
                case COM_SHOP:
                    c = 'S';
                    break;
                case COM_CAFE:
                    c = 'C';
                    break;
                case TRANSPORT:
                    c = 'T';
                    break;
                case PUBLIC:
                    c = 'P';
                    break;
                case LANDSCAPE:
                    c = 'L';
                    break;
                case ROAD:
                    c = 'D';
                    break;
                default:
                    c = '.';
                    break;
                }
                cout << c << ' ';
            }
            cout << endl;
        }
    }

    // Function to color the grid mesh based on types
    void zTsConfiguratorLanduse::colorGridFaces(const vector<vector<CellType>>& grid)
    {
        for (int i = 0; i < ROWS; ++i)
        {
            for (int j = 0; j < COLS; ++j)
            {
                zItMeshFace f(displayMesh, gridFaceIds[i][j]);

                switch (grid[i][j])
                {
                case RESIDENTIAL:
                    f.setColor(zColor(1, 0.3, 0, 1));
                    break;
                case OFFICE:
                    f.setColor(zColor(0, 1, 1, 1));
                    break;
                case COM_SHOP:
                    f.setColor(zColor(0, 0.3, 1, 1));
                    break;
                case COM_CAFE:
                    f.setColor(zColor(1, 1, 0, 1));
                    break;
                case TRANSPORT:
                    f.setColor(zColor(1, 0, 0, 1));
                    break;
                case PUBLIC:
                    f.setColor(zColor(1, 0, 1, 1));
                    break;
                case LANDSCAPE:
                    f.setColor(zColor(0, 1, 0, 1));
                    break;
                case ROAD:
                    f.setColor(zColor(0, 0, 0, 1));
                    break;
                default:
                    f.setColor(zColor(1, 1, 1, 1));
                    break;
                }
            }
        }
    }


    // Function to compute distance maps for each land use type
    map<CellType, vector<vector<double>>> zTsConfiguratorLanduse::computeDistanceMaps(const vector<vector<CellType>>& grid)
    {
        map<CellType, vector<vector<double>>> distanceMaps;

        for (CellType landUseType : landUseTypes)
        {
            vector<vector<double>> distanceMap(ROWS, vector<double>(COLS, numeric_limits<double>::max()));
            queue<pair<int, int>> q;

            // Initialize the queue with positions of the land use type
            for (int i = 0; i < ROWS; ++i)
            {
                for (int j = 0; j < COLS; ++j)
                {
                    if (grid[i][j] == landUseType)
                    {
                        distanceMap[i][j] = 0.0;
                        q.emplace(i, j);
                    }
                }
            }

            // BFS to compute distances
            while (!q.empty())
            {
                int x = q.front().first;
                int y = q.front().second;
                q.pop();

                static const int dx[] = { -1, 0, 1, 0 };
                static const int dy[] = { 0, 1, 0, -1 };

                for (int dir = 0; dir < 4; ++dir)
                {
                    int nx = x + dx[dir];
                    int ny = y + dy[dir];

                    if (nx >= 0 && nx < ROWS && ny >= 0 && ny < COLS)
                    {
                        if (distanceMap[nx][ny] > distanceMap[x][y] + 1.0)
                        {
                            distanceMap[nx][ny] = distanceMap[x][y] + 1.0;
                            q.emplace(nx, ny);
                        }
                    }
                }
            }

            distanceMaps[landUseType] = distanceMap;
        }

        return distanceMaps;
    }

    // Function to calculate total score based on distances to land use types
    double zTsConfiguratorLanduse::calculateScore(const vector<vector<CellType>>& grid, const map<CellType, vector<vector<double>>>& distanceMaps)
    {
        double totalScore = 0.0;

        // Calculate score for each agent cell
        for (int i = 0; i < ROWS; ++i)
        {
            for (int j = 0; j < COLS; ++j)
            {
                CellType agentCell = grid[i][j];
                if (find(agentTypes.begin(), agentTypes.end(), agentCell) == agentTypes.end())
                {
                    continue; // Not an agent cell
                }

                const vector<float>& preferences = agentPreferences[agentCell];
                double agentScore = 0.0;

                // For each land use type
                for (size_t k = 0; k < landUseTypes.size(); ++k)
                {
                    CellType landUseType = landUseTypes[k];
                    float preference = preferences[k];

                    double distance = distanceMaps.at(landUseType)[i][j];

                    // Avoid division by zero and check if distance is finite
                    if (distance > 0.0 && distance < numeric_limits<double>::max())
                    {
                        agentScore += preference / distance;
                    }
                }

                totalScore += agentScore;
            }
        }

        return totalScore;
    }

    // Simulated Annealing Optimisation
    void zTsConfiguratorLanduse::optimiseGrid(
        vector<vector<CellType>>& grid,
        const map<CellType, vector<vector<double>>>& distanceMaps,
        double _temperature,
        double _cooldown,
        double _coolingRate)
    {
        double temperature = _temperature;
        double cooldown = _cooldown;
        double coolingRate = _coolingRate;
        double currentScore = calculateScore(grid, distanceMaps);
        vector<vector<CellType>> bestGrid = grid;
        double bestScore = currentScore;

        int iteration = 0;

        auto optimisationStart = high_resolution_clock::now();

        // Logging headers for performance analysis
        LOG_PERF << "| Iteration | Temperature | Current Score | Best Score | Time (ms) |\n";
        LOG_PERF << "|-----------|-------------|---------------|------------|-----------|\n";

        while (temperature > cooldown)
        {
            // Generate neighboring solution by swapping two random agent cells
            vector<vector<CellType>> newGrid = grid;
            int x1, y1, x2, y2;

            // Find first agent cell to swap
            do
            {
                x1 = rand() % ROWS;
                y1 = rand() % COLS;
            } while (find(agentTypes.begin(), agentTypes.end(), grid[x1][y1]) == agentTypes.end());

            // Find second agent cell to swap
            do
            {
                x2 = rand() % ROWS;
                y2 = rand() % COLS;
            } while (find(agentTypes.begin(), agentTypes.end(), grid[x2][y2]) == agentTypes.end());

            // Swap the agent cells
            swap(newGrid[x1][y1], newGrid[x2][y2]);

            double newScore = calculateScore(newGrid, distanceMaps);
            double deltaScore = newScore - currentScore;

            /*
            The probability of accepting a new configuration is determined by the Metropolis criterion:

            𝑃 = 𝑒^(Δ𝑆/𝑇)

            ΔS: Change in score (new score minus current score).

            Positive ΔS: The new configuration is better and is always accepted.
            Negative ΔS: The new configuration is worse; acceptance depends on the temperature.
            T: Current temperature.
            */
            if (deltaScore > 0 || exp(deltaScore / temperature) > ((double)rand() / RAND_MAX))
            {
                grid = newGrid;
                currentScore = newScore;

                if (currentScore > bestScore)
                {
                    bestGrid = grid;
                    bestScore = currentScore;
                }
            }

            // Logging for performance analysis every 100 iterations
            if (iteration % 100 == 0)
            {
                auto now = high_resolution_clock::now();
                auto duration = duration_cast<milliseconds>(now - optimisationStart);
                LOG_PERF << "| " << setw(9) << iteration
                    << " | " << setw(11) << fixed << setprecision(2) << temperature
                    << " | " << setw(13) << currentScore
                    << " | " << setw(10) << bestScore
                    << " | " << setw(9) << duration.count()
                    << " |\n";

                colorGridFaces(grid);
                finalIteration = iteration;
            }

            iteration++;
            temperature *= 1 - coolingRate;
        }

        grid = bestGrid;
    }

    void zTsConfiguratorLanduse::draw()
    {
        displayMesh.draw();
    }

    // Generate grid with random land use and agents based on percentages
    void zTsConfiguratorLanduse::generateGrid_random(vector<vector<CellType>>& grid, map<CellType, double> agentPercentages)
    {
        // Initialize grid with EMPTY cells
        for (int i = 0; i < ROWS; ++i)
            for (int j = 0; j < COLS; ++j)
                grid[i][j] = EMPTY;

        // Set preset land use types (fixed cells)
        int numLandUseCells = (ROWS * COLS) / 5; // Adjust as needed

        for (CellType landUseType : landUseTypes)
        {
            for (int i = 0; i < numLandUseCells / landUseTypes.size(); ++i)
            {
                int x, y;
                do
                {
                    x = rand() % ROWS;
                    y = rand() % COLS;
                } while (grid[x][y] != EMPTY);
                grid[x][y] = landUseType;
            }
        }

        // Calculate the number of each agent type to place
        int totalCells = ROWS * COLS;
        int fixedCells = 0;
        for (int i = 0; i < ROWS; ++i)
            for (int j = 0; j < COLS; ++j)
                if (grid[i][j] != EMPTY)
                    fixedCells++;

        int availableCells = totalCells - fixedCells;
        map<CellType, int> agentCounts;
        for (CellType agentType : agentTypes)
        {
            agentCounts[agentType] = static_cast<int>(agentPercentages[agentType] * availableCells);
        }

        // Fill the grid with agent types
        vector<CellType> agentsToPlace;
        for (CellType agentType : agentTypes)
        {
            agentsToPlace.insert(agentsToPlace.end(), agentCounts[agentType], agentType);
        }

        // Shuffle and assign to grid
        //random_shuffle(agentsToPlace.begin(), agentsToPlace.end());
        std::random_device rd;                        // Seed generator
        std::mt19937 g(rd());                         // Mersenne Twister engine
		shuffle(agentsToPlace.begin(), agentsToPlace.end(), g);

        int index = 0;
        for (int i = 0; i < ROWS && index < agentsToPlace.size(); ++i)
        {
            for (int j = 0; j < COLS && index < agentsToPlace.size(); ++j)
            {
                if (grid[i][j] == EMPTY)
                {
                    grid[i][j] = agentsToPlace[index++];
                }
            }
        }
    }

    // Generate grid based on input grid and assign agents to empty cells
    void zTsConfiguratorLanduse::generateGrid_input(vector<vector<CellType>>& grid, map<CellType, double> agentPercentages)
    {
        // Count fixed cells
        int totalCells = ROWS * COLS;
        int fixedCells = 0;
        vector<pair<int, int>> emptyCells;
        for (int i = 0; i < ROWS; ++i)
        {
            for (int j = 0; j < COLS; ++j)
            {
                if (grid[i][j] != EMPTY && find(landUseTypes.begin(), landUseTypes.end(), grid[i][j]) == landUseTypes.end())
                {
                    // If cell is not empty and not a land use type, consider it fixed
                    fixedCells++;
                }
                else if (grid[i][j] == EMPTY)
                {
                    emptyCells.emplace_back(i, j);
                }
            }
        }

        int availableCells = emptyCells.size();
        map<CellType, int> agentCounts;
        for (CellType agentType : agentTypes)
        {
            agentCounts[agentType] = static_cast<int>(agentPercentages[agentType] * availableCells);
        }

        // Fill the empty cells with agent types
        vector<CellType> agentsToPlace;
        for (CellType agentType : agentTypes)
        {
            agentsToPlace.insert(agentsToPlace.end(), agentCounts[agentType], agentType);
        }

        // In case of rounding errors, fill remaining cells with random agent types
        while (agentsToPlace.size() < availableCells)
        {
            agentsToPlace.push_back(agentTypes[rand() % agentTypes.size()]);
        }

        // Shuffle and assign to grid
        //random_shuffle(agentsToPlace.begin(), agentsToPlace.end());
        std::random_device rd;                        // Seed generator
        std::mt19937 g(rd());                         // Mersenne Twister engine
        shuffle(agentsToPlace.begin(), agentsToPlace.end(), g);
        int index = 0;
        for (auto& cell : emptyCells)
        {
            grid[cell.first][cell.second] = agentsToPlace[index++];
        }
    }

    // Generate grid with heuristic-based agent placement on input grid
    void zTsConfiguratorLanduse::generateGrid_heuristic(vector<vector<CellType>>& grid, map<CellType, double> agentPercentages, const map<CellType, vector<vector<double>>>& distanceMaps)
    {
        // Identify empty cells
        vector<pair<int, int>> emptyCells;
        for (int i = 0; i < ROWS; ++i)
            for (int j = 0; j < COLS; ++j)
                if (grid[i][j] == EMPTY)
                    emptyCells.emplace_back(i, j);

        int availableCells = emptyCells.size();

        // Calculate the number of each agent type to place
        map<CellType, int> agentCounts;
        int totalAgents = 0;
        for (CellType agentType : agentTypes)
        {
            int count = static_cast<int>(agentPercentages[agentType] * availableCells);
            agentCounts[agentType] = count;
            totalAgents += count;
        }

        // Adjust for any rounding errors
        while (totalAgents < availableCells)
        {
            // Assign remaining cells to random agent types
            CellType randomAgent = agentTypes[rand() % agentTypes.size()];
            agentCounts[randomAgent]++;
            totalAgents++;
        }

        // For each agent type, calculate preference scores for all empty cells
        map<CellType, vector<tuple<double, int, int>>> agentCellScores;
        for (CellType agentType : agentTypes)
        {
            const vector<float>& preferences = agentPreferences[agentType];
            vector<tuple<double, int, int>> cellScores; // (score, x, y)

            for (const auto& cell : emptyCells)
            {
                int i = cell.first;
                int j = cell.second;
                double score = 0.0;

                // Calculate preference score for this cell
                for (size_t k = 0; k < landUseTypes.size(); ++k)
                {
                    CellType landUseType = landUseTypes[k];
                    float preference = preferences[k];
                    double distance = distanceMaps.at(landUseType)[i][j];

                    if (distance > 0.0 && distance < numeric_limits<double>::max())
                    {
                        score += preference / distance;
                    }
                }

                cellScores.emplace_back(-score, i, j); // Negative score for descending sort
            }

            // Sort cells based on scores (descending)
            sort(cellScores.begin(), cellScores.end());

            agentCellScores[agentType] = cellScores;
        }

        // Assign agents to cells based on highest preference scores
        vector<vector<bool>> occupied(ROWS, vector<bool>(COLS, false));
        for (const auto& cell : emptyCells)
        {
            occupied[cell.first][cell.second] = false;
        }

        for (CellType agentType : agentTypes)
        {
            int agentsToPlace = agentCounts[agentType];
            vector<tuple<double, int, int>>& cellScores = agentCellScores[agentType];
            int placedAgents = 0;

            for (const auto& entry : cellScores)
            {
                if (placedAgents >= agentsToPlace)
                    break;

                double score = -get<0>(entry); // Original score
                int i = get<1>(entry);
                int j = get<2>(entry);

                if (!occupied[i][j])
                {
                    grid[i][j] = agentType;
                    occupied[i][j] = true;
                    placedAgents++;
                }
            }

            // If not all agents placed (due to conflicts), assign randomly
            while (placedAgents < agentsToPlace)
            {
                int idx = rand() % emptyCells.size();
                int i = emptyCells[idx].first;
                int j = emptyCells[idx].second;

                if (!occupied[i][j])
                {
                    grid[i][j] = agentType;
                    occupied[i][j] = true;
                    placedAgents++;
                }
            }
        }
    }

}
