#include "switches.h"
#include "util.h"
#include "physics.h"
#include <cstring>
bool canUseLift1(int fromFloor, int toFloor)
{
    if (fromFloor == toFloor)
        return false;
    return (fromFloor == 1 && toFloor == 2) ||
           (fromFloor == 2 && toFloor == 1);
}
bool canUseLift2(int fromFloor, int toFloor,
                 bool lift2Operational, int emergencyTripsLeft)
{
    bool validFromFloor =
        (fromFloor == 2 || fromFloor == 4 || fromFloor == 5);
    bool validToFloor =
        (toFloor == 2 || toFloor == 4 || toFloor == 5);
    if (!validFromFloor || !validToFloor || fromFloor == toFloor)
        return false;
    if (lift2Operational)
        return true;
    return emergencyTripsLeft > 0;
}
bool unlockFloor3Door(const char inventory[MAX_INVENTORY_CAPACITY],
                      int inventoryCount)
{
    return hasItem(inventory, inventoryCount, ENT_MASTER_KEY);
}
bool installFuseControlPanel(char inventory[MAX_INVENTORY_CAPACITY],
                             int &inventoryCount,
                             bool objectives[NUM_OBJECTIVES],
                             int &score)
{
    if (inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY)
        return false;
    if (objectives[OBJ_FUSE_INSTALLED])
        return false;
    if (!objectives[OBJ_FUSE_COLLECTED])
        return false;
    if (!hasItem(inventory, inventoryCount, ENT_FUSE))
        return false;
    if (!removeItem(inventory, inventoryCount, ENT_FUSE))
        return false;
    objectives[OBJ_FUSE_INSTALLED] = true;
    objectives[OBJ_LIFT2_ACTIVE] = true;
    applyScoreDelta(score, 20);
    return true;
}
bool verifyCabinetCode(const char *enteredCode,
                       const bool objectives[NUM_OBJECTIVES])
{
    if (enteredCode == nullptr)
        return false;
    if (!objectives[OBJ_CLUE_4] ||
        !objectives[OBJ_CLUE_2] ||
        !objectives[OBJ_CLUE_7])
        return false;
    if (objectives[OBJ_CABINET_UNLOCKED])
        return false;
    return strcmp(enteredCode, "427") == 0;
}

// Door lock mechanics, lifts, and cabinet verification without structures
