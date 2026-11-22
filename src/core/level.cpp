#include "level.h"
#include "util.h"
#include <iostream>
#include <string>
#include <fstream>
using namespace std;
static bool parseNumber(const char *str, int &outValue)
{
    if (str == nullptr || *str == '\0')
    {
        return false;
    }
    int result = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return false;
        }
        result = result * 10 + (str[i] - '0');
    }
    outValue = result;
    return true;
}
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
bool loadInitialState(const char *filename,
                      int &playerFloor, int &playerRoom,
                      int &energy, int &time, int &score,
                      char inventory[MAX_INVENTORY_CAPACITY],
                      int &inventoryCount,
                      bool objectives[NUM_OBJECTIVES],
                      int &emergencyTripsLeft)
{
    energy = INITIAL_ENERGY;
    time = INITIAL_TIME;
    score = INITIAL_SCORE;
    playerFloor = 1;
    playerRoom = 1;

    inventoryCount = 0;
    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
    {
        inventory[i] = ENT_NONE;
    }
    emergencyTripsLeft = 0;

    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        objectives[i] = false;
    }
    ifstream file(filename);

    if (!file)
    {
        cout << "Error: Could not open " << filename << endl;
        return false;
    }
    string line;
    while (getline(file, line))
    {
        size_t colonPos = line.find(':');

        if (colonPos == string::npos)
        {
            continue;
        }

        size_t keyLength = colonPos;
        size_t valueLength = line.length() - colonPos - 1;

        if (keyLength >= MAX_LINE_LEN || valueLength >= MAX_LINE_LEN)
        {
            cout << "Error: Line too long in initial state file." << endl;
            return false;
        }

        char key[MAX_LINE_LEN];
        char value[MAX_LINE_LEN];

        for (size_t i = 0; i < keyLength; i++)
        {
            key[i] = line[i];
        }
        key[keyLength] = '\0';

        for (size_t i = 0; i < valueLength; i++)
        {
            value[i] = line[colonPos + 1 + i];
        }
        value[valueLength] = '\0';

        trimWhitespace(key);
        trimWhitespace(value);
        if (stringEquals(key, "INITIAL_ENERGY"))
        {
            int result = 0;
            if (!parseNumber(value, result))
            {
                cout << "Error: Invalid initial energy value: " << value << endl;
                return false;
            }
            energy = result;
        }
        else if (stringEquals(key, "INITIAL_TIME"))
        {
            int result = 0;
            if (!parseNumber(value, result))
            {
                cout << "Error: Invalid initial time value: " << value << endl;
                return false;
            }
            time = result;
        }
        else if (stringEquals(key, "INITIAL_SCORE"))
        {
            int result = 0;

            if (!parseNumber(value, result))
            {
                cout << "Error: Invalid initial score value: " << value << endl;
                return false;
            }
            score = result;
        }
        else if (stringEquals(key, "INITIAL_INVENTORY"))
        {
            if (stringEquals(value, "NONE"))
            {
                inventoryCount = 0;
            }
            else
            {
                cout << "Error: Invalid initial inventory value: " << value << endl;
                return false;
            }
        }
        else if (stringEquals(key, "PASSENGER_RESCUED"))
        {
            if (stringEquals(value, "YES"))
            {
                objectives[OBJ_PASSENGER_RESCUED] = true;
            }
        }
        else if (stringEquals(key, "CAT_RESCUED"))
        {
            if (stringEquals(value, "YES"))
            {
                objectives[OBJ_CAT_SECURED] = true;
            }
        }
        else if (stringEquals(key, "CABINET_UNLOCKED"))
        {
            if (stringEquals(value, "YES"))
            {
                objectives[OBJ_CABINET_UNLOCKED] = true;
            }
        }
        else if (stringEquals(key, "TRAIN_KEY_COLLECTED"))
        {
            if (stringEquals(value, "YES"))
            {
                objectives[OBJ_TRAIN_KEY] = true;
            }
        }
        else if (stringEquals(key, "FUSE_INSTALLED"))
        {
            if (stringEquals(value, "YES"))
            {
                objectives[OBJ_FUSE_INSTALLED] = true;
            }
        }
        else if (stringEquals(key, "LIFT_PASS_COLLECTED"))
        {
            if (stringEquals(value, "YES"))
            {
                objectives[OBJ_LIFT_PASS] = true;
            }
        }
        else if (stringEquals(key, "TRAIN_PREPARED"))
        {
            if (stringEquals(value, "YES"))
            {
                objectives[OBJ_TRAIN_PREPARED] = true;
            }
        }
    }
    return true;
}

bool validateStartingRoom(int startFloor, int startRoom,
                          const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])
{
    // Validate floor and room numbers
    if (startFloor < 1 || startFloor > NUM_FLOORS ||
        startRoom < 1 || startRoom > ROOMS_PER_FLOOR)
    {
        return false;
    }

    bool visited[NUM_FLOORS][ROOMS_PER_FLOOR] = {false};

    visited[startFloor - 1][startRoom - 1] = true;

    bool changed = true;

    // Check reachable rooms without using Lift 2
    while (changed)
    {
        changed = false;

        for (int f = 0; f < NUM_FLOORS; f++)
        {
            for (int r = 0; r < ROOMS_PER_FLOOR; r++)
            {
                if (!visited[f][r])
                {
                    continue;
                }

                // Check left connection
                if (r > 0)
                {
                    char connection = horizontalConn[f][r - 1];

                    if ((connection == CONN_DOOR ||
                         connection == CONN_WINDOW) &&
                        !visited[f][r - 1])
                    {
                        visited[f][r - 1] = true;
                        changed = true;
                    }
                }

                // Check right connection
                if (r < ROOMS_PER_FLOOR - 1)
                {
                    char connection = horizontalConn[f][r];

                    if ((connection == CONN_DOOR ||
                         connection == CONN_WINDOW) &&
                        !visited[f][r + 1])
                    {
                        visited[f][r + 1] = true;
                        changed = true;
                    }
                }

                // Staircase 1: F1-R1 <-> F2-R1
                if (f == 0 && r == 0 && !visited[1][0])
                {
                    visited[1][0] = true;
                    changed = true;
                }
                else if (f == 1 && r == 0 && !visited[0][0])
                {
                    visited[0][0] = true;
                    changed = true;
                }

                // Staircase 2: F2-R1 <-> F3-R1
                if (f == 1 && r == 0 && !visited[2][0])
                {
                    visited[2][0] = true;
                    changed = true;
                }
                else if (f == 2 && r == 0 && !visited[1][0])
                {
                    visited[1][0] = true;
                    changed = true;
                }

                // Staircase 3: F4-R1 <-> F5-R1
                if (f == 3 && r == 0 && !visited[4][0])
                {
                    visited[4][0] = true;
                    changed = true;
                }
                else if (f == 4 && r == 0 && !visited[3][0])
                {
                    visited[3][0] = true;
                    changed = true;
                }

                // Lift 1: F1-R2/R3 <-> F2-R2/R3
                if (f == 0 && (r == 1 || r == 2) &&
                    !visited[1][r])
                {
                    visited[1][r] = true;
                    changed = true;
                }
                else if (f == 1 && (r == 1 || r == 2) &&
                         !visited[0][r])
                {
                    visited[0][r] = true;
                    changed = true;
                }
            }
        }
    }
    int count = 0;
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            if (visited[f][r])
            {
                count++;
            }
        }
    }
    return count > 1;
}
const char *getEntityName(char entitySymbol)
{
    if (entitySymbol == ENT_MASTER_KEY)
        return "Master Key";
    else if (entitySymbol == ENT_ENERGY_PACK)
        return "Energy Pack";
    else if (entitySymbol == ENT_EMERGENCY_TOOLKIT)
        return "Emergency Toolkit";
    else if (entitySymbol == ENT_CLUE_4)
        return "Clue 4";
    else if (entitySymbol == ENT_FUSE)
        return "Fuse";
    else if (entitySymbol == ENT_CAT)
        return "Cat";
    else if (entitySymbol == ENT_CONTROL_PANEL)
        return "Control Panel";
    else if (entitySymbol == ENT_LIFT_PASS)
        return "Lift Pass";
    else if (entitySymbol == ENT_PASSENGER)
        return "Passenger";
    else if (entitySymbol == ENT_MEDICAL_KIT)
        return "Medical Kit";
    else if (entitySymbol == ENT_EMERGENCY_CARD)
        return "Emergency Card";
    else if (entitySymbol == ENT_CLUE_2)
        return "Clue 2";
    else if (entitySymbol == ENT_CLUE_7)
        return "Clue 7";
    else if (entitySymbol == ENT_CABINET_TRAIN_KEY)
        return "Cabinet Train Key";
    else if (entitySymbol == ENT_CAT_CARRIER)
        return "Cat Carrier";
    else if (entitySymbol == ENT_OBSERVATION_TOKEN)
        return "Observation Token";
    else
        return "Nothing";
}
// Modular implementation using primitive arrays for map parsing and verification
