
#include "simulator.h"
#include "level.h"
#include "logger.h"
#include "viewer_iface.h"
#include <iostream>
#include <limits>

using namespace std;

int main()
{
    int playerFloor = 1;
    int playerRoom = 1;
    int energy = INITIAL_ENERGY;
    int time = INITIAL_TIME;
    int score = INITIAL_SCORE;

    char inventory[MAX_INVENTORY_CAPACITY];
    int inventoryCount = 0;
    int emergencyTripsLeft = 0;

    bool objectives[NUM_OBJECTIVES] = {false};

    char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1];
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM];
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR] = {0};

    const char *mapFile = "data/building.txt";
    const char *initStateFile = "data/initial-state.txt";

    if (!loadBuildingMap(mapFile, horizontalConn, roomEntities, entityCounts))
    {
        cerr << "Error: Could not load " << mapFile << "\n";
        return 1;
    }

    if (!loadInitialState(initStateFile, playerFloor, playerRoom,
                          energy, time, score, inventory, inventoryCount,
                          objectives, emergencyTripsLeft))
    {
        cerr << "Error: Could not load " << initStateFile << "\n";
        return 1;
    }

    cout << "===========================================\n";
    cout << " THE LAST TRAIN - SIMULATION INITIALIZED   \n";
    cout << "===========================================\n";

    cout << "\nSelect your starting location.\n";

    bool validStart = false;

    while (!validStart)
    {
        cout << "Enter starting Floor (1-" << NUM_FLOORS
             << ") and Room (1-" << ROOMS_PER_FLOOR << "): ";

        if (!(cin >> playerFloor >> playerRoom))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter numbers only.\n";
            continue;
        }

        if (validateStartingRoom(playerFloor, playerRoom, horizontalConn))
        {
            validStart = true;
        }
        else
        {
            cout << "Invalid or inaccessible starting room. Try again.\n";
        }
    }

    cout << "\nStarting location selected: Floor "
         << playerFloor << ", Room " << playerRoom << "\n";

    runSimulation(playerFloor, playerRoom, energy, time, score,
                  inventory, inventoryCount, objectives, emergencyTripsLeft,
                  horizontalConn, roomEntities, entityCounts);

    return 0;
}
