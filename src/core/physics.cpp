#include "physics.h"
#include "util.h"

void applyResourceDelta(int &energy, int &time, int deltaEnergy, int deltaTime)
{
    energy = clampInt(energy + deltaEnergy, 0, MAX_ENERGY);
    time = clampInt(time + deltaTime, 0, MAX_TIME);
}

void applyScoreDelta(int &score, int deltaScore)
{
    score += deltaScore;

    if (score < 0)
    {
        score = 0;
    }
}
bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY],
             int inventoryCount, char itemSymbol)
{
    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }
    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
        {
            return true;
        }
    }

    return false;
}
bool addItem(char inventory[MAX_INVENTORY_CAPACITY],
             int &inventoryCount, char itemSymbol)
{
    if (inventoryCount >= MAX_INVENTORY_CAPACITY)
    {
        return false;
    }
    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }
    if (hasItem(inventory, inventoryCount, itemSymbol))
    {
        return false;
    }

    inventory[inventoryCount] = itemSymbol;
    inventoryCount++;

    return true;
}
bool removeItem(char inventory[MAX_INVENTORY_CAPACITY],
                int &inventoryCount, char itemSymbol)
{
    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }
    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
        {
            for (int j = i; j < inventoryCount - 1; j++)
            {
                inventory[j] = inventory[j + 1];
            }

            inventoryCount--;

            return true;
        }
    }

    return false;
}
bool isDirectlyConnected(int fromF, int fromR, int toF, int toR,
                         const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                         char &outConnType)
{
    outConnType = CONN_NONE;

    if (fromF < 0 || fromF >= NUM_FLOORS ||
        toF < 0 || toF >= NUM_FLOORS ||
        fromR < 0 || fromR >= ROOMS_PER_FLOOR ||
        toR < 0 || toR >= ROOMS_PER_FLOOR)
    {
        return false;
    }

    if (fromF != toF)
    {
        return false;
    }

    if (fromR - toR != 1 && toR - fromR != 1)
    {
        return false;
    }

    int connectionIndex;

    if (fromR < toR)
    {
        connectionIndex = fromR;
    }
    else
    {
        connectionIndex = toR;
    }

    char connection = horizontalConn[fromF][connectionIndex];

    if (connection != CONN_DOOR &&
        connection != CONN_WINDOW &&
        connection != CONN_LOCKED_DOOR)
    {
        return false;
    }
    outConnType = connection;

    return true;
}
bool canTraverse(int targetFloor, int targetRoom, char connType,
                 const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                 int emergencyTripsLeft, bool lift2Operational)
{
    if (targetFloor < 0 || targetFloor >= NUM_FLOORS ||
        targetRoom < 0 || targetRoom >= ROOMS_PER_FLOOR)
    {
        return false;
    }

    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }
    if (connType == CONN_DOOR)
    {
        return true;
    }
    if (connType == CONN_WINDOW)
    {
        return true;
    }
    if (connType == CONN_LOCKED_DOOR)
    {
        return hasItem(inventory, inventoryCount, ENT_MASTER_KEY);
    }
    if (connType == 'S')
    {
        return true;
    }

    if (connType == '1')
    {
        return true;
    }
    if (connType == '2')
    {
        if (lift2Operational)
        {
            return hasItem(inventory, inventoryCount, ENT_LIFT_PASS);
        }

        if (emergencyTripsLeft > 0 &&
            hasItem(inventory, inventoryCount, ENT_EMERGENCY_CARD))
        {
            return true;
        }

        return false;
    }

    return false;
}