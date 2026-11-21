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
    return true;

}

// Modular implementation using primitive arrays for map parsing and verification
