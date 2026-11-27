
#include "simulator.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "level.h"
#include "config.h"
#include "viewer_iface.h"
#include <iostream>
#include <string>
#include <limits>

using namespace std;

bool handleMoveCommand(int targetFloor, int targetRoom,
                       int &currentFloor, int &currentRoom,
                       int &energy, int &time,
                       const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                       const char inventory[MAX_INVENTORY_CAPACITY],
                       int inventoryCount,
                       int &emergencyTripsLeft,
                       bool lift2Operational)
{
    if (targetFloor < 1 || targetFloor > NUM_FLOORS ||
        targetRoom < 1 || targetRoom > ROOMS_PER_FLOOR ||
        currentFloor < 1 || currentFloor > NUM_FLOORS ||
        currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY ||
        energy <= 0 || time <= 0 ||
        emergencyTripsLeft < 0)
    {
        return false;
    }

    if (targetFloor == currentFloor &&
        targetRoom == currentRoom)
    {
        return false;
    }

    int fromF = currentFloor - 1;
    int fromR = currentRoom - 1;
    int toF = targetFloor - 1;
    int toR = targetRoom - 1;

    char connType = CONN_NONE;
    bool emergencyTrip = false;

    if (fromF == toF)
    {
        if (!isDirectlyConnected(fromF, fromR, toF, toR,
                                 horizontalConn, connType))
        {
            return false;
        }
    }
    else
    {
        bool staircase1 =
            (fromF == 0 && fromR == 0 && toF == 1 && toR == 0) ||
            (fromF == 1 && fromR == 0 && toF == 0 && toR == 0);

        bool staircase2 =
            (fromF == 1 && fromR == 0 && toF == 2 && toR == 0) ||
            (fromF == 2 && fromR == 0 && toF == 1 && toR == 0);

        bool staircase3 =
            (fromF == 3 && fromR == 0 && toF == 4 && toR == 0) ||
            (fromF == 4 && fromR == 0 && toF == 3 && toR == 0);

        bool lift1 =
            fromR == toR &&
            (fromR == 1 || fromR == 2) &&
            canUseLift1(fromF, toF);

        bool lift2 =
            fromR == 3 &&
            toR == 3 &&
            canUseLift2(fromF, toF, lift2Operational,
                        emergencyTripsLeft);

        if (staircase1 || staircase2 || staircase3)
        {
            connType = 'S';
        }
        else if (lift1)
        {
            connType = '1';
        }
        else if (lift2)
        {
            connType = '2';
            emergencyTrip = !lift2Operational;
        }
        else
        {
            return false;
        }
    }

    if (!canTraverse(toF, toR, connType,
                     inventory, inventoryCount,
                     emergencyTripsLeft, lift2Operational))
    {
        return false;
    }

    if (connType == CONN_WINDOW)
    {
        applyResourceDelta(energy, time,
                           COST_MOVE_WINDOW_ENERGY,
                           COST_MOVE_WINDOW_TIME);
    }
    else if (connType == '1' || connType == '2')
    {
        applyResourceDelta(energy, time,
                           COST_MOVE_LIFT_ENERGY,
                           COST_MOVE_LIFT_TIME);
    }
    else
    {
        applyResourceDelta(energy, time,
                           COST_MOVE_ORDINARY_ENERGY,
                           COST_MOVE_ORDINARY_TIME);
    }

    currentFloor = targetFloor;
    currentRoom = targetRoom;

    if (emergencyTrip)
    {
        emergencyTripsLeft--;
    }

    return true;
}

bool handleInspectCommand(int currentFloor, int currentRoom,
                          int &energy, int &time,
                          const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int entityCount,
                          bool objectives[NUM_OBJECTIVES],
                          int &score)
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||
        currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR ||
        entityCount < 0 ||
        entityCount > MAX_ENTITIES_PER_ROOM ||
        energy <= 0 || time <= 0)
    {
        return false;
    }

    int f = currentFloor - 1;
    int r = currentRoom - 1;

    applyResourceDelta(energy, time,
                       COST_INSPECT_ENERGY,
                       COST_INSPECT_TIME);

    bool clueFound = false;

    for (int i = 0; i < entityCount; i++)
    {
        char symbol = roomEntities[f][r][i];
        int objectiveIndex = -1;

        if (symbol == ENT_CLUE_4)
        {
            objectiveIndex = OBJ_CLUE_4;
        }
        else if (symbol == ENT_CLUE_2)
        {
            objectiveIndex = OBJ_CLUE_2;
        }
        else if (symbol == ENT_CLUE_7)
        {
            objectiveIndex = OBJ_CLUE_7;
        }

        if (objectiveIndex != -1 &&
            !objectives[objectiveIndex])
        {
            objectives[objectiveIndex] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            clueFound = true;
        }
    }

    if (clueFound)
    {
        cout << "New clue discovered! +10 points.\n";
    }
    else
    {
        cout << "Inspection completed. No new clues found.\n";
    }

    return true;
}

bool handleCollectCommand(char itemSymbol,
                          int currentFloor, int currentRoom,
                          int &energy, int &time, int &score,
                          char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int &entityCount,
                          char inventory[MAX_INVENTORY_CAPACITY],
                          int &inventoryCount,
                          bool objectives[NUM_OBJECTIVES])
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||
        currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR ||
        entityCount < 0 ||
        entityCount > MAX_ENTITIES_PER_ROOM ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY ||
        energy <= 0 || time <= 0)
    {
        return false;
    }

    // Only valid collectible entities are allowed.
    bool validCollectible =
        itemSymbol == ENT_MASTER_KEY ||
        itemSymbol == ENT_ENERGY_PACK ||
        itemSymbol == ENT_EMERGENCY_TOOLKIT ||
        itemSymbol == ENT_FUSE ||
        itemSymbol == ENT_LIFT_PASS ||
        itemSymbol == ENT_PASSENGER ||
        itemSymbol == ENT_CAT ||
        itemSymbol == ENT_MEDICAL_KIT ||
        itemSymbol == ENT_EMERGENCY_CARD ||
        itemSymbol == ENT_CAT_CARRIER ||
        itemSymbol == ENT_OBSERVATION_TOKEN;

    if (!validCollectible)
    {
        return false;
    }

    int f = currentFloor - 1;
    int r = currentRoom - 1;
    int entityIndex = -1;

    // Find the requested item in the current room.
    for (int i = 0; i < entityCount; i++)
    {
        if (roomEntities[f][r][i] == itemSymbol)
        {
            entityIndex = i;
            break;
        }
    }

    if (entityIndex == -1)
    {
        return false;
    }

    bool specialBonus =
        itemSymbol == ENT_ENERGY_PACK ||
        itemSymbol == ENT_EMERGENCY_TOOLKIT ||
        itemSymbol == ENT_OBSERVATION_TOKEN;

    bool rescuePassenger = (itemSymbol == ENT_PASSENGER);
    bool rescueCat = (itemSymbol == ENT_CAT);

    // Check passenger rescue requirements.
    if (rescuePassenger)
    {
        if (!hasItem(inventory, inventoryCount, ENT_MEDICAL_KIT) ||
            objectives[OBJ_PASSENGER_RESCUED])
        {
            return false;
        }
    }

    // Check cat rescue requirements.
    if (rescueCat)
    {
        if (!hasItem(inventory, inventoryCount, ENT_CAT_CARRIER) ||
            objectives[OBJ_CAT_SECURED])
        {
            return false;
        }
    }

    // Prevent collecting already completed objectives.
    if (itemSymbol == ENT_MASTER_KEY &&
        objectives[OBJ_MASTER_KEY])
    {
        return false;
    }

    if (itemSymbol == ENT_FUSE &&
        objectives[OBJ_FUSE_COLLECTED])
    {
        return false;
    }

    if (itemSymbol == ENT_LIFT_PASS &&
        objectives[OBJ_LIFT_PASS])
    {
        return false;
    }

    if (itemSymbol == ENT_CAT_CARRIER &&
        objectives[OBJ_CAT_CARRIER])
    {
        return false;
    }

    // Prevent duplicate inventory items.
    // Bonus items and rescued entities are not stored in inventory.
    if (!specialBonus && !rescuePassenger && !rescueCat)
    {
        if (hasItem(inventory, inventoryCount, itemSymbol) ||
            inventoryCount >= MAX_INVENTORY_CAPACITY)
        {
            return false;
        }

        if (!addItem(inventory, inventoryCount, itemSymbol))
        {
            return false;
        }
    }

    // Apply the pickup cost.
    applyResourceDelta(energy, time,
                       COST_ITEM_PICKUP_ENERGY,
                       COST_ITEM_PICKUP_TIME);

    // Apply item-specific effects.
    if (itemSymbol == ENT_ENERGY_PACK)
    {
        applyResourceDelta(energy, time,
                           BONUS_ENERGY_PACK, 0);
    }
    else if (itemSymbol == ENT_EMERGENCY_TOOLKIT)
    {
        applyResourceDelta(energy, time,
                           BONUS_EMERGENCY_TOOLKIT, 0);
    }
    else if (itemSymbol == ENT_OBSERVATION_TOKEN)
    {
        applyResourceDelta(energy, time,
                           0, BONUS_OBSERVATION_TOKEN);
    }
    else if (itemSymbol == ENT_MASTER_KEY)
    {
        objectives[OBJ_MASTER_KEY] = true;
        applyScoreDelta(score, SCORE_KEY_COLLECT);
    }
    else if (itemSymbol == ENT_FUSE)
    {
        objectives[OBJ_FUSE_COLLECTED] = true;
    }
    else if (itemSymbol == ENT_LIFT_PASS)
    {
        objectives[OBJ_LIFT_PASS] = true;
        applyScoreDelta(score, SCORE_LIFT_PASS);
    }
    else if (itemSymbol == ENT_CAT_CARRIER)
    {
        objectives[OBJ_CAT_CARRIER] = true;
    }
    else if (itemSymbol == ENT_PASSENGER)
    {
        objectives[OBJ_PASSENGER_RESCUED] = true;
        applyScoreDelta(score, SCORE_RESCUE_PASSENGER);
    }
    else if (itemSymbol == ENT_CAT)
    {
        objectives[OBJ_CAT_SECURED] = true;
        applyScoreDelta(score, SCORE_SECURE_CAT);
    }

    // Remove the collected entity from the room.
    for (int i = entityIndex; i < entityCount - 1; i++)
    {
        roomEntities[f][r][i] = roomEntities[f][r][i + 1];
    }

    entityCount--;
    roomEntities[f][r][entityCount] = ENT_NONE;

    cout << getEntityName(itemSymbol)
         << " collected successfully.\n";

    return true;
}

bool handleUseCommand(char itemSymbol, int currentFloor, int currentRoom,
                      int &energy, int &time, int &score,
                      char inventory[MAX_INVENTORY_CAPACITY],
                      int &inventoryCount,
                      bool objectives[NUM_OBJECTIVES])
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||
        currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY ||
        energy <= 0 || time <= 0)
    {
        return false;
    }

    // Fuse can only be installed at Floor 3, Room 4.
    if (currentFloor != 3 || currentRoom != 4 ||
        itemSymbol != ENT_FUSE)
    {
        return false;
    }

    if (!installFuseControlPanel(inventory, inventoryCount,
                                 objectives, score))
    {
        return false;
    }

    applyResourceDelta(energy, time,
                       COST_ITEM_USE_ENERGY,
                       COST_ITEM_USE_TIME);

    cout << "Fuse installed successfully. Lift 2 is now operational.\n";

    return true;
}

bool handleUnlockCabinetCommand(const char *code,
                                int currentFloor, int currentRoom,
                                bool objectives[NUM_OBJECTIVES],
                                int &score,
                                char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                                int &entityCount)
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||
        currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR ||
        entityCount < 0 ||
        entityCount > MAX_ENTITIES_PER_ROOM ||
        code == nullptr)
    {
        return false;
    }

    // Cabinet is located at Floor 5, Room 1.
    if (currentFloor != 5 || currentRoom != 1)
        return false;

    if (objectives[OBJ_CABINET_UNLOCKED] ||
        objectives[OBJ_TRAIN_KEY])
    {
        return false;
    }

    // Find the cabinet in the current room.
    int cabinetIndex = -1;

    for (int i = 0; i < entityCount; i++)
    {
        if (roomEntities[4][0][i] == ENT_CABINET_TRAIN_KEY)
        {
            cabinetIndex = i;
            break;
        }
    }

    if (cabinetIndex == -1)
        return false;

    // Verify the entered code and all three clues.
    if (!verifyCabinetCode(code, objectives))
    {
        cout << "Incorrect code or clues incomplete.\n";
        return false;
    }

    objectives[OBJ_CABINET_UNLOCKED] = true;
    objectives[OBJ_TRAIN_KEY] = true;

    applyScoreDelta(score, SCORE_CABINET_UNLOCK);
    applyScoreDelta(score, SCORE_TRAIN_KEY);

    // Remove the cabinet entity after unlocking.
    for (int i = cabinetIndex; i < entityCount - 1; i++)
    {
        roomEntities[4][0][i] = roomEntities[4][0][i + 1];
    }

    entityCount--;
    roomEntities[4][0][entityCount] = ENT_NONE;

    cout << "Cabinet unlocked! Train Key obtained. +40 points.\n";

    return true;
}
bool checkEndConditions(int energy, int time,
                        const bool objectives[NUM_OBJECTIVES])
{
    // Mission fails if energy or time reaches zero.
    if (energy <= 0 || time <= 0)
        return true;

    // Mission ends when victory or failure is recorded.
    if (objectives[OBJ_MISSION_WON] ||
        objectives[OBJ_MISSION_FAILED])
    {
        return true;
    }

    return false;
}

void runSimulation(int &playerFloor, int &playerRoom,
                   int &energy, int &time, int &score,
                   char inventory[MAX_INVENTORY_CAPACITY],
                   int &inventoryCount,
                   bool objectives[NUM_OBJECTIVES],
                   int &emergencyTripsLeft,
                   char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                   char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                   int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    const char *saveFile = "./out/savegame.txt";
    const char *summaryFile = "./out/game_summary.txt";

    bool gameRunning = true;
    bool victory = false;

    cout << "\n====================================\n";
    cout << "       THE LAST TRAIN\n";
    cout << "====================================\n";
    cout << "Mission started. Good luck!\n";
    cout << "Type 'help' to see available commands.\n";

    while (gameRunning)
    {
        // Check whether the mission has already ended.
        if (checkEndConditions(energy, time, objectives))
        {
            if (energy <= 0 || time <= 0)
            {
                objectives[OBJ_MISSION_FAILED] = true;
                cout << "\nMISSION FAILED! Energy or time exhausted.\n";
            }

            victory = objectives[OBJ_MISSION_WON];
            break;
        }

        // Display the current game information.
        cout << "\n------------------------------------\n";

        renderDashboard(playerFloor, playerRoom,
                        energy, time, score,
                        inventory, inventoryCount,
                        emergencyTripsLeft);

        renderAsciiBuilding(playerFloor, playerRoom);

        renderRoomDetails(playerFloor - 1, playerRoom - 1,
                          roomEntities,
                          entityCounts[playerFloor - 1][playerRoom - 1]);

        renderObjectivesChecklist(objectives);

        cout << "\nEnter command: ";

        string command;

        if (!(cin >> command))
        {
            cout << "\nInput ended. Exiting game.\n";
            break;
        }

        if (command == "help")
        {
            cout << "\nAVAILABLE COMMANDS\n";
            cout << "move <floor> <room>  - Move to another room\n";
            cout << "inspect              - Inspect current room\n";
            cout << "collect <symbol>     - Collect an item\n";
            cout << "use <symbol>         - Use an item\n";
            cout << "unlock <code>        - Unlock the cabinet\n";
            cout << "save                 - Save game progress\n";
            cout << "load                 - Load saved progress\n";
            cout << "status               - Display game status\n";
            cout << "map                  - Display building map\n";
            cout << "objectives           - Display objectives\n";
            cout << "quit                 - Exit the game\n";
        }
        else if (command == "move")
        {
            int targetFloor, targetRoom;

            if (!(cin >> targetFloor >> targetRoom))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid movement input.\n";
                continue;
            }

            if (handleMoveCommand(
                    targetFloor, targetRoom,
                    playerFloor, playerRoom,
                    energy, time,
                    horizontalConn,
                    inventory, inventoryCount,
                    emergencyTripsLeft,
                    objectives[OBJ_LIFT2_ACTIVE]))
            {
                cout << "Movement successful.\n";
            }
            else
            {
                cout << "Movement not possible. Check connection and requirements.\n";
            }
        }
        else if (command == "inspect")
        {
            handleInspectCommand(
                playerFloor, playerRoom,
                energy, time,
                roomEntities,
                entityCounts[playerFloor - 1][playerRoom - 1],
                objectives, score);
        }
        else if (command == "collect")
        {
            char itemSymbol;

            if (!(cin >> itemSymbol))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid item symbol.\n";
                continue;
            }

            if (handleCollectCommand(
                    itemSymbol,
                    playerFloor, playerRoom,
                    energy, time, score,
                    roomEntities,
                    entityCounts[playerFloor - 1][playerRoom - 1],
                    inventory, inventoryCount,
                    objectives))
            {
                if (itemSymbol == ENT_EMERGENCY_CARD)
                {
                    emergencyTripsLeft = 3;
                    cout << "Emergency Lift 2 access: 3 trips available.\n";
                }
            }
            else
            {
                cout << "Collection failed. Item unavailable or requirements not met.\n";
            }
        }
        else if (command == "use")
        {
            char itemSymbol;

            if (!(cin >> itemSymbol))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid item symbol.\n";
                continue;
            }

            if (!handleUseCommand(
                    itemSymbol,
                    playerFloor, playerRoom,
                    energy, time, score,
                    inventory, inventoryCount,
                    objectives))
            {
                cout << "Cannot use this item here.\n";
            }
        }
        else if (command == "unlock")
        {
            string code;

            if (!(cin >> code))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid cabinet code.\n";
                continue;
            }

            if (!handleUnlockCabinetCommand(
                    code.c_str(),
                    playerFloor, playerRoom,
                    objectives, score,
                    roomEntities,
                    entityCounts[playerFloor - 1][playerRoom - 1]))
            {
                cout << "Cabinet could not be unlocked.\n";
            }
        }
        else if (command == "save")
        {
            if (saveGameStateToFile(
                    saveFile,
                    playerFloor, playerRoom,
                    energy, time, score,
                    inventory, inventoryCount,
                    objectives, emergencyTripsLeft,
                    roomEntities, entityCounts))
            {
                cout << "Game saved successfully.\n";
            }
            else
            {
                cout << "Unable to save game.\n";
            }
        }
        else if (command == "load")
        {
            if (loadGameStateFromFile(
                    saveFile,
                    playerFloor, playerRoom,
                    energy, time, score,
                    inventory, inventoryCount,
                    objectives, emergencyTripsLeft,
                    roomEntities, entityCounts))
            {
                cout << "Saved game loaded successfully.\n";
            }
            else
            {
                cout << "Unable to load saved game.\n";
            }
        }
        else if (command == "status")
        {
            renderDashboard(playerFloor, playerRoom,
                            energy, time, score,
                            inventory, inventoryCount,
                            emergencyTripsLeft);
        }
        else if (command == "map")
        {
            renderAsciiBuilding(playerFloor, playerRoom);
        }
        else if (command == "objectives")
        {
            renderObjectivesChecklist(objectives);
        }
        else if (command == "quit")
        {
            cout << "Exiting game. Goodbye!\n";
            gameRunning = false;
        }
        else
        {
            cout << "Unknown command. Type 'help' for commands.\n";
        }

        // Check resource failure after every command.
        if (energy <= 0 || time <= 0)
        {
            objectives[OBJ_MISSION_FAILED] = true;
            cout << "\nMISSION FAILED! Energy or time exhausted.\n";
            gameRunning = false;
            break;
        }

        // Check whether the player has completed all mission objectives
        // and reached the final train room.
        bool missionRequirementsMet =
            objectives[OBJ_TRAIN_KEY] &&
            objectives[OBJ_PASSENGER_RESCUED] &&
            objectives[OBJ_CAT_SECURED] &&
            objectives[OBJ_FUSE_INSTALLED] &&
            objectives[OBJ_LIFT_PASS];

        if (playerFloor == 5 && playerRoom == 2 &&
            missionRequirementsMet &&
            !objectives[OBJ_MISSION_WON])
        {
            objectives[OBJ_TRAIN_PREPARED] = true;
            applyScoreDelta(score, SCORE_PREPARE_TRAIN);

            objectives[OBJ_MISSION_WON] = true;
            applyScoreDelta(score, SCORE_MISSION_VICTORY);

            victory = true;
            gameRunning = false;

            cout << "\n====================================\n";
            cout << "       MISSION COMPLETED!\n";
            cout << "====================================\n";
            cout << "The train is prepared and boarded.\n";
            cout << "Congratulations! You rescued everyone.\n";
        }
    }

    // Write the final mission report.
    if (objectives[OBJ_MISSION_WON] ||
        objectives[OBJ_MISSION_FAILED])
    {
        writeGameSummaryReport(
            summaryFile,
            score, energy, time,
            objectives, victory);

        cout << "\nFinal score: " << score << "\n";
        cout << "Game summary saved to " << summaryFile << "\n";
    }
}
