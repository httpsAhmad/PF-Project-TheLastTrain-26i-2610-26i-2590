#include "switches.h"
#include "physics.h"
#include <cstring>
bool canUseLift1(int fromFloor, int toFloor)
{
    if (fromFloor == toFloor)
        return false;
    return (fromFloor == 0 && toFloor == 1) ||
           (fromFloor == 1 && toFloor == 0);
}
bool canUseLift2(int fromFloor, int toFloor,
                 bool lift2Operational, int emergencyTripsLeft)
{
    bool validFromFloor =
        (fromFloor == 1 || fromFloor == 3 || fromFloor == 4);
    bool validToFloor =
        (toFloor == 1 || toFloor == 3 || toFloor == 4);
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
