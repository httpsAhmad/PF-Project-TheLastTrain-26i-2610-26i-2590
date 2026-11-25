#include "util.h"
#include "viewer_iface.h"
#include <iostream>
#include "level.h"
using namespace std;
void renderDashboard(int playerFloor, int playerRoom,
                     int energy, int time, int score,
                     const char inventory[MAX_INVENTORY_CAPACITY],
                     int inventoryCount, int emergencyTripsLeft)
{
    cout << "\n====================================\n";
    cout << "          THE LAST TRAIN\n";
    cout << "====================================\n";
    cout << "Player Location : Floor " << playerFloor + 1
         << ", Room " << playerRoom + 1 << '\n';
    cout << "Energy          : " << energy
         << "/" << MAX_ENERGY << '\n';
    cout << "Time            : " << time
         << "/" << MAX_TIME << '\n';
    cout << "Score           : " << score << '\n';
    cout << "Emergency Trips : " << emergencyTripsLeft << '\n';
    cout << "\n---------- INVENTORY ----------\n";
    if (inventoryCount <= 0)
    {
        cout << "Inventory is empty.\n";
    }
    else if (inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        cout << "Invalid inventory count.\n";
    }
    else
    {
        for (int i = 0; i < inventoryCount; i++)
        {
            cout << i + 1 << ". "
                 << getEntityName(inventory[i])
                 << " (" << inventory[i] << ")\n";
        }
    }
    cout << "====================================\n";
}
void renderRoomDetails(
    int floor, int room,
    const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCount)
{
    cout << "\n---------- ROOM DETAILS ----------\n";
    if (floor < 0 || floor >= NUM_FLOORS ||
        room < 0 || room >= ROOMS_PER_FLOOR)
    {
        cout << "Invalid room location.\n";
        return;
    }
    cout << "Floor: " << floor + 1
         << " | Room: " << room + 1 << '\n';
    if (entityCount <= 0)
    {
        cout << "No entities in this room.\n";
        return;
    }
    if (entityCount > MAX_ENTITIES_PER_ROOM)
    {
        cout << "Invalid entity count.\n";
        return;
    }
    cout << "Entities in this room:\n";
    for (int i = 0; i < entityCount; i++)
    {
        char entity = roomEntities[floor][room][i];

        if (entity != ENT_NONE)
        {
            cout << "- " << getEntityName(entity)
                 << " (" << entity << ")\n";
        }
    }
}
void renderAsciiBuilding(int playerFloor, int playerRoom)
{
    cout << "\n========== BUILDING MAP ==========\n";
    if (playerFloor < 0 || playerFloor >= NUM_FLOORS ||
        playerRoom < 0 || playerRoom >= ROOMS_PER_FLOOR)
    {
        cout << "Invalid player location.\n";
        return;
    }
    for (int floor = 0; floor < NUM_FLOORS; floor++)
    {
        cout << "\nFloor " << floor + 1 << ":\n";
        for (int room = 0; room < ROOMS_PER_FLOOR; room++)
        {
            if (floor == playerFloor && room == playerRoom)
            {
                cout << "[ P ] ";
            }
            else
            {
                cout << "[ R" << room + 1 << " ] ";
            }
        }
        cout << '\n';
    }
    cout << "\nP = Player's Current Room\n";
    cout << "==================================\n";
}
void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES])
{
    const char *objectiveNames[NUM_OBJECTIVES] =
        {
            "Collect Master Key",
            "Find Clue 4",
            "Find Clue 2",
            "Find Clue 7",
            "Unlock Cabinet",
            "Collect Train Key",
            "Collect Fuse",
            "Install Fuse",
            "Collect Lift Pass",
            "Activate Lift 2",
            "Rescue Passenger",
            "Collect Cat Carrier",
            "Secure Cat",
            "Prepare Train",
            "Win Mission",
            "Mission Failed"};
    cout << "\n========== OBJECTIVES ==========\n";
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        if (objectives[i])
        {
            cout << "[X] " << objectiveNames[i] << '\n';
        }
        else
        {
            cout << "[ ] " << objectiveNames[i] << '\n';
        }
    }
    cout << "================================\n";
}
// Terminal UI and dashboard rendering using primitive parameters
