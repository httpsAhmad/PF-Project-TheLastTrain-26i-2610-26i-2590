#include "level.h"
#include "util.h"
#include <iostream>
#include <string>
#include <fstream>
using namespace std;
bool loadBuildingMap(const char *filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    ifstream file(filename);
    if (!file)
    {
        cout << "Error: Could not open " << filename << endl;
        return false;
    }
    for (int floor = 0; floor < NUM_FLOORS; ++floor)
    {
        for (int room = 0; room < ROOMS_PER_FLOOR - 1; ++room)
        {
            horizontalConn[floor][room] = CONN_NONE;
        }
    }
    for (int floor = 0; floor < NUM_FLOORS; ++floor)
    {
        for (int room = 0; room < ROOMS_PER_FLOOR; ++room)
        {
            entityCounts[floor][room] = 0;
            for (int entity = 0; entity < MAX_ENTITIES_PER_ROOM; ++entity)
            {
                roomEntities[floor][room][entity] = ENT_NONE;
            }
        }
    }
    string line;
    while (getline(file, line))
    {
        if (line.length() >= 57 && line[10] == 'F' && line[13] == 'R')
        {
            int floorNum = line[11] - '0';
            if (floorNum < 1 || floorNum > NUM_FLOORS)
            {
                cout << "Error: Invalid floor number in line: " << line << endl;
                return false;
            }
            for (int c = 0; c < ROOMS_PER_FLOOR - 1; c++)
            {
                int col = 20 + 12 * c;
                char x = line[col];
                if (x == CONN_DOOR || x == CONN_WINDOW || x == CONN_LOCKED_DOOR || x == CONN_SOLID_WALL)
                {
                    horizontalConn[floorNum - 1][c] = x;
                }
            }
            string itemLine;
            if (getline(file, itemLine) && itemLine.length() >= 57)
            {
                for (int r = 0; r < ROOMS_PER_FLOOR; r++)
                {
                    int startCol = 9 + 12 * r;
                    for (int col = startCol; col <= startCol + 10; col++)
                    {
                        char ch = itemLine[col];
                        if (ch != ' ' && entityCounts[floorNum - 1][r] < MAX_ENTITIES_PER_ROOM)
                        {
                            roomEntities[floorNum - 1][r][entityCounts[floorNum - 1][r]] = ch;
                            entityCounts[floorNum - 1][r]++;
                        }
                    }
                }
            }
        }
    }

    return true;
}

// Modular implementation using primitive arrays for map parsing and verification
