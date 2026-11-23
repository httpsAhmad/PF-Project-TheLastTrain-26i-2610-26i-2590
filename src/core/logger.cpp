#include "logger.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;
bool saveGameStateToFile(const char *filename,
                         int playerFloor, int playerRoom,
                         int energy, int time, int score,
                         const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                         const bool objectives[NUM_OBJECTIVES], int emergencyTripsLeft,
                         const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                         const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    if (filename == nullptr || inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY ||
        playerFloor < 0 || playerFloor >= NUM_FLOORS ||
        playerRoom < 0 || playerRoom >= ROOMS_PER_FLOOR ||
        energy < 0 || energy > MAX_ENERGY ||
        time < 0 || time > MAX_TIME ||
        score < 0 || emergencyTripsLeft < 0 || emergencyTripsLeft > 3)
    {
        return false;
    }
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            if (entityCounts[f][r] < 0 ||
                entityCounts[f][r] > MAX_ENTITIES_PER_ROOM)
            {
                return false;
            }
        }
    }
    ofstream file(filename);
    if (!file)
    {
        return false;
    }
    file << "THE_LAST_TRAIN_SAVE_V1\n";
    file << playerFloor << " "
         << playerRoom << " "
         << energy << " "
         << time << " "
         << score << " "
         << inventoryCount << " "
         << emergencyTripsLeft << "\n";
    for (int i = 0; i < inventoryCount; i++)
    {
        file << static_cast<int>(static_cast<unsigned char>(inventory[i])) << " ";
    }
    file << "\n";
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        file << (objectives[i] ? 1 : 0) << " ";
    }
    file << "\n";
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            file << entityCounts[f][r] << " ";
            for (int e = 0; e < entityCounts[f][r]; e++)
            {
                file << static_cast<int>(
                            static_cast<unsigned char>(roomEntities[f][r][e]))
                     << " ";
            }
            file << "\n";
        }
    }
    file.close();
    return !file.fail();
}
bool loadGameStateFromFile(const char *filename,
                           int &playerFloor, int &playerRoom,
                           int &energy, int &time, int &score,
                           char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount,
                           bool objectives[NUM_OBJECTIVES], int &emergencyTripsLeft,
                           char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                           int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    if (filename == nullptr)
    {
        return false;
    }
    ifstream file(filename);
    if (!file)
    {
        return false;
    }
    char fileHeader[32];
    file >> fileHeader;
    if (!file || strcmp(fileHeader, "THE_LAST_TRAIN_SAVE_V1") != 0)
    {
        return false;
    }
    int tempFloor, tempRoom, tempEnergy, tempTime;
    int tempScore, tempInventoryCount, tempEmergencyTrips;
    file >> tempFloor >> tempRoom >> tempEnergy >> tempTime >> tempScore >> tempInventoryCount >> tempEmergencyTrips;
    if (!file ||
        tempFloor < 0 || tempFloor >= NUM_FLOORS ||
        tempRoom < 0 || tempRoom >= ROOMS_PER_FLOOR ||
        tempEnergy < 0 || tempEnergy > MAX_ENERGY ||
        tempTime < 0 || tempTime > MAX_TIME ||
        tempScore < 0 ||
        tempInventoryCount < 0 ||
        tempInventoryCount > MAX_INVENTORY_CAPACITY ||
        tempEmergencyTrips < 0 || tempEmergencyTrips > 3)
    {
        return false;
    }
    char tempInventory[MAX_INVENTORY_CAPACITY] = {};
    for (int i = 0; i < tempInventoryCount; i++)
    {
        int symbol;
        file >> symbol;
        if (!file || symbol < 0 || symbol > 255)
        {
            return false;
        }
        tempInventory[i] = static_cast<char>(symbol);
    }
    bool tempObjectives[NUM_OBJECTIVES];
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        int value;
        file >> value;
        if (!file || (value != 0 && value != 1))
        {
            return false;
        }
        tempObjectives[i] = (value == 1);
    }
    char tempRoomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM] = {};
    int tempEntityCounts[NUM_FLOORS][ROOMS_PER_FLOOR] = {};
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            file >> tempEntityCounts[f][r];
            if (!file ||
                tempEntityCounts[f][r] < 0 ||
                tempEntityCounts[f][r] > MAX_ENTITIES_PER_ROOM)
            {
                return false;
            }
            for (int e = 0; e < tempEntityCounts[f][r]; e++)
            {
                int symbol;
                file >> symbol;
                if (!file || symbol < 0 || symbol > 255)
                {
                    return false;
                }
                tempRoomEntities[f][r][e] = static_cast<char>(symbol);
            }
        }
    }
    if (!file)
    {
        return false;
    }
    playerFloor = tempFloor;
    playerRoom = tempRoom;
    energy = tempEnergy;
    time = tempTime;
    score = tempScore;
    inventoryCount = tempInventoryCount;
    emergencyTripsLeft = tempEmergencyTrips;
    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
    {
        inventory[i] = tempInventory[i];
    }
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        objectives[i] = tempObjectives[i];
    }
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            entityCounts[f][r] = tempEntityCounts[f][r];
            for (int e = 0; e < MAX_ENTITIES_PER_ROOM; e++)
            {
                roomEntities[f][r][e] = tempRoomEntities[f][r][e];
            }
        }
    }

    return true;
}
void writeGameSummaryReport(const char *filename,
                            int score, int remainingEnergy, int remainingTime,
                            const bool objectives[NUM_OBJECTIVES], bool victory)
{
    if (filename == nullptr)
    {
        return;
    }
    ofstream file(filename);
    if (!file)
    {
        return;
    }
    const char *objectiveNames[NUM_OBJECTIVES] =
        {
            "Master Key Collected",
            "Clue 4 Found",
            "Clue 2 Found",
            "Clue 7 Found",
            "Cabinet Unlocked",
            "Train Key Collected",
            "Fuse Collected",
            "Fuse Installed",
            "Lift Pass Collected",
            "Lift 2 Activated",
            "Passenger Rescued",
            "Cat Carrier Collected",
            "Cat Secured",
            "Train Prepared",
            "Mission Won",
            "Mission Failed"};
    file << "====================================\n";
    file << "       THE LAST TRAIN REPORT\n";
    file << "====================================\n\n";
    file << "Mission Result: "
         << (victory ? "VICTORY" : "FAILURE") << "\n";
    file << "Final Score: " << score << "\n";
    file << "Remaining Energy: " << remainingEnergy << "\n";
    file << "Remaining Time: " << remainingTime << "\n\n";
    file << "OBJECTIVES\n";
    file << "------------------------------------\n";
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        file << objectiveNames[i] << ": "
             << (objectives[i] ? "Completed" : "Not Completed")
             << "\n";
    }
    file << "\n====================================\n";
    file << "           END OF REPORT\n";
    file << "====================================\n";
}
// File persistence for savegame.txt and game_summary.txt using primitive variables
