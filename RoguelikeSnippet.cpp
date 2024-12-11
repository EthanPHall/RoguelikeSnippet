#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <limits>
#include <algorithm>

using std::cout;
using std::cin;
using std::string;
using std::array;
using std::unique_ptr;
using std::make_unique;
using std::vector;
// using std::move; // I choose not to use this specific name because "move" is such generic word and I don't want to potentially run into an ambiguous names issue.

class Actor; //Base class for all Actors, objects that simulate characters, creatures, and any other entity that has agency in how it acts
class IActorFactory; //Interface for Actor factories
class PredefinedActorFactory; //Actor Factory implementation that creates and returns actors defined by the factory itself
enum class EnemyType { Goblin };
class Enemy; 
class Player;

class IItem;
class IItemFactory;
class PredefinedItemFactory;
enum class ItemType { Sword, HealthPotion };
class IWeapon;
class IActiveBuff;
class Sword;
class HealthPotion;
class Inventory;

class Room; //Base class for all rooms
class IRoomFactory; 
class IRoomVisitor; //Classes derived from Room have important data that some other classes need to access, so implementing the Visitor pattern is mostly about avoiding casts
class PredefinedRoomFactory;//Implementation of IRoomFactory. Returns Room objects defined by the factory itself based on provided RoomTypes
enum class RoomType { Enemy };
class EnemyRoom; //Spawns in an enemy. The player must defeat the the enemy to move on, or game over.
class GameMap; //Handles generating the next room, keeping track of the current room, and getting data from the current room.

class GameplayManager; //Runs the main game loop, takes in input, and keep track of progress and whether or not to move on or end the game.
enum class GameplayStatus {Ongoing, Victory, Gameover};
class GameDataManager;
class IRenderer;
class TextRenderer;
class AsciiRenderer;
class IActionHandler;
class ConsoleActionHandler;


class Actor{
public:
    Actor(string name, int hp, int strength, int speed, int agility) : name(name), hp(hp), strength(strength), speed(speed), agility(agility) {
        inventory = make_unique<Inventory>();
    }
    string getName() { return name; }
    int getHp() { return hp; }
    void modHp(int modifier) { hp += modifier; }
    int getStrength() { return strength; }
    int getSpeed() { return speed; }
    int getAgility() { return agility; }
    Inventory* getInventory() { return inventory.get(); }
private:
    string name;
    int hp;
    int strength;
    int speed;
    int agility;

    unique_ptr<Inventory> inventory;
};

class Enemy : public Actor{
public:
    Enemy(string name, int hp, int strength, int speed, int agility, EnemyType enemyType) : Actor(name, hp, strength, speed, agility), enemyType(enemyType) {}
    EnemyType getEnemyType() { return enemyType; }
private:
    EnemyType enemyType;
};

class Player : public Actor{
public:
    Player(string name, int hp, int strength, int speed, int agility) : Actor(name, hp, strength, speed, agility) {}
private:
};

class IActorFactory{
public:
    //TODO: Look into the rule of 3 and or Rule of 5, I think this violates those principles.
    virtual ~IActorFactory() = default;
    virtual unique_ptr<Enemy> createEnemy(EnemyType type) const = 0;
    virtual unique_ptr<Player> createPlayer() const = 0;
private:
};



class IItemFactory{
public:
    virtual ~IItemFactory() = default;
    virtual void createAndStoreItem(Inventory* toStoreIn, ItemType type) = 0;
private:
};



class PredefinedActorFactory: public IActorFactory{
public:
    PredefinedActorFactory(IItemFactory* itemFactory): itemFactory(itemFactory){}

    unique_ptr<Enemy> createEnemy(EnemyType type) const{
        switch(type){
            case EnemyType::Goblin:
            {
                unique_ptr<Enemy> newGoblin = make_unique<Enemy>("Goblin", 10, 1, 1, 2, EnemyType::Goblin);

                //Equip the Goblin with a starter Weapon
                itemFactory->createAndStoreItem(newGoblin->getInventory(), ItemType::Sword);
                return std::move(newGoblin);
            }
            default:
            {
                //TODO: Even if for no other enemy type, turn creating a goblin into its own function
                
                unique_ptr<Enemy> newGoblin = make_unique<Enemy>("Goblin", 10, 1, 1, 2, EnemyType::Goblin);

                //Equip the Goblin with a starter Weapon
                itemFactory->createAndStoreItem(newGoblin->getInventory(), ItemType::Sword);
                return std::move(newGoblin);
            }
        }
    }

    unique_ptr<Player> createPlayer() const{
        unique_ptr<Player> newPlayer = make_unique<Player>("Player", 100, 1, 1, 1);

        //Equip the Player with a starter Weapon
        itemFactory->createAndStoreItem(newPlayer->getInventory(), ItemType::Sword);

        //Equip the Player with a health potion
        itemFactory->createAndStoreItem(newPlayer->getInventory(), ItemType::HealthPotion);


        return std::move(newPlayer);
    }

private:
    IItemFactory* itemFactory;
};





class IItem{
public:
    virtual string getName() = 0;
    virtual bool isSingleUse() = 0;
    virtual ItemType getType() = 0;
private:
};

class IWeapon : public IItem{
public:
    virtual void applyDamage(Actor& user, Actor& target) = 0;
    virtual string getName() = 0;
    virtual bool isSingleUse() = 0;
    virtual ItemType getType() = 0;
    virtual int getExpectedDamage(Actor& user) = 0;
private:
};

class IActiveBuff : public IItem{
public:
    virtual void applyBuff(Actor& target) = 0;
    virtual string getName() = 0;
    virtual string getBuffAction() = 0;
    virtual bool isSingleUse() = 0;
    virtual ItemType getType() = 0;
    virtual int getMagnitude() = 0;
private:
};

class Sword : public IWeapon{
public:
    Sword(string name, int damage, ItemType itemType) : name(name), damage(damage), itemType(itemType) {}
    string getName(){ return name; }
    void applyDamage(Actor& user, Actor& target){
        int damageToDeal = getExpectedDamage(user);
        target.modHp(-damageToDeal);
    }
    bool isSingleUse(){
        return false;
    }
    ItemType getType() {
        return itemType;
    }
    int getExpectedDamage(Actor& user){
        int damageToDeal = damage + user.getAgility() + user.getStrength();
        return damageToDeal;
    }
private:
    string name;
    int damage;
    ItemType itemType;
};

class HealthPotion : public IActiveBuff{
public:
    HealthPotion(string name, int healAmount, ItemType itemType) : name(name), healAmount(healAmount), itemType(itemType) {}
    string getName(){
        return name;
    }
    string getBuffAction(){
        return "Heal";
    }
    void applyBuff(Actor& target){
        target.modHp(healAmount);
    }
    bool isSingleUse(){
        return true;
    }
    ItemType getType(){
        return itemType;
    }
    int getMagnitude(){
        return healAmount;
    }
private:
    string name;
    int healAmount;
    ItemType itemType;
};

class Inventory{
public:
    Inventory(){}

    vector<IWeapon*> getWeapons() {
        vector<IWeapon*> weaponPointers;
        for(auto& weapon: weapons){
            weaponPointers.push_back(weapon.get());
        }

        return weaponPointers;
    }
    vector<IActiveBuff*> getActiveBuffs() {
        vector<IActiveBuff*> buffPointers;
        for(auto& buff: activeBuffs){
            buffPointers.push_back(buff.get());
        }

        return buffPointers;
    }

    void transferWeaponOwnership(unique_ptr<IWeapon> newWeapon){
        weapons.push_back(std::move(newWeapon));
    }
    void transferActiveBuffOwnership(unique_ptr<IActiveBuff> newBuff){
        activeBuffs.push_back(std::move(newBuff));
    }
    void removeItem(IItem* toRemove){
        switch(toRemove->getType()){
            case ItemType::Sword:{
                removeWeapon(toRemove);
            }
            case ItemType::HealthPotion:{
                removeActiveBuff(toRemove);
            }
        }
    }

    int getTotalItems(){
        return weapons.size() + activeBuffs.size();
    }
private:
    vector<unique_ptr<IWeapon>> weapons;
    vector<unique_ptr<IActiveBuff>> activeBuffs;

    //TODO: Simplify the remove methods here with a function that can handle any uniquePointer<IItem> vector. Maybe a template?
    void removeWeapon(IItem* toRemove){
        //Try to find the first weapon that shares a name with the one to remove.
        auto weaponsIt = 
            std::find_if(
                weapons.begin(), 
                weapons.end(), 
                [toRemove](unique_ptr<IWeapon>& weaponInList) { return weaponInList->getName() == toRemove->getName(); }
            );

        //If the weapon was found, erase it.
        if(weaponsIt != weapons.end()){
            weapons.erase(weaponsIt);
        }
    }
    void removeActiveBuff(IItem* toRemove){
        //Try to find the first activeBuff that shares a name with the one to remove.
        auto buffsIt = 
            std::find_if(
                activeBuffs.begin(), 
                activeBuffs.end(), 
                [toRemove](unique_ptr<IActiveBuff>& buffInList) { return buffInList->getName() == toRemove->getName(); }
            );

        //If the weapon was found, erase it.
        if(buffsIt != activeBuffs.end()){
            activeBuffs.erase(buffsIt);
        }
    }
};

class PredefinedItemFactory: public IItemFactory{
public:
    void createAndStoreItem(Inventory* toStoreIn, ItemType type){
        switch(type){
            case ItemType::Sword:{
                unique_ptr<Sword> newSword = make_unique<Sword>("Sword", 3, ItemType::Sword);
                toStoreIn->transferWeaponOwnership(std::move(newSword));
                break;
            }
            case ItemType::HealthPotion:{
                unique_ptr<HealthPotion> newPotion = make_unique<HealthPotion>("Health Potion", 20, ItemType::HealthPotion);
                toStoreIn->transferActiveBuffOwnership(std::move(newPotion));
                break;
            }
            default:{
                //Do Nothing
            }
        }
    }
private:
};





class Room{
public:
    Room(string name, RoomType type, array<RoomType, 3> neighbors) : name(name), type(type), neighbors(neighbors) {}
    virtual void accept(IRoomVisitor& visitor) = 0;
    virtual bool isCleared() = 0;
    RoomType getNeighbor(int index) { return neighbors[index]; }
    int getNeighborsCount() { return neighbors.size(); }
    string getName() { return name; }
    RoomType getRoomType() { return type; }
private:
    string name;
    RoomType type;
    //neighbors are the rooms that can be visited from this room, so the previous room is not included
    array<RoomType, 3> neighbors;
};

class IRoomVisitor{
public:
    virtual void visit(EnemyRoom& enemyRoom) = 0;
};

class EnemyRoom: public Room{
public:
    EnemyRoom(string name, RoomType type, array<RoomType, 3> neighbors, const IActorFactory* actorFactory): Room(name, type, neighbors), actorFactory(actorFactory) {
        enemy = std::move(actorFactory->createEnemy(EnemyType::Goblin));
    }

    void accept(IRoomVisitor& visitor) override{
        visitor.visit(*this);
    }

    bool isCleared() override{
        return enemy->getHp() <= 0;
    }

    Enemy* getEnemy() { return enemy.get(); }

private:
    const IActorFactory* actorFactory;
    unique_ptr<Enemy> enemy;
};

class IRoomFactory{
public:
    virtual ~IRoomFactory() = default;
    virtual unique_ptr<Room> createRoom(RoomType type) = 0;
};

class PredefinedRoomFactory : public IRoomFactory{
public:
    PredefinedRoomFactory(const IActorFactory* actorFactory): actorFactory(actorFactory){}
    unique_ptr<Room> createRoom(RoomType type){
        switch(type){
            case RoomType::Enemy:
            {
                array<RoomType, 3> neighbors {RoomType::Enemy, RoomType::Enemy, RoomType::Enemy};
                return std::move(make_unique<EnemyRoom>("Enemy Room", RoomType::Enemy, neighbors, actorFactory));
            }
            default:
            {
                array<RoomType, 3> neighbors {RoomType::Enemy, RoomType::Enemy, RoomType::Enemy};
                return std::move(make_unique<EnemyRoom>("Enemy Room", RoomType::Enemy, neighbors, actorFactory));
            }
        }
    }
private:
    const IActorFactory* actorFactory;
};

class GameMap{
public:
    GameMap(IRoomFactory* roomFactory): roomFactory(std::move(roomFactory)){
        //Determine the type of the first room of the map
        //TODO: Determine this randomly
        RoomType newType = RoomType::Enemy;

        //Generate the new Room
        currentRoom = std::move(roomFactory->createRoom(newType));
    }
    string getCurrentRoomName(){ return currentRoom->getName(); }
    int getCurrentRoomNeighborCount(){ return currentRoom->getNeighborsCount(); }
    RoomType getCurrentRoomType(){ return currentRoom->getRoomType(); }
    void currentRoomAcceptVisitor(IRoomVisitor& visitor) { currentRoom->accept(visitor); }
    bool isCurrentRoomCleared() { return currentRoom->isCleared(); }

    void moveToNextRoom(){
        //Figure out what room type the new room needs to be
        //TODO: Determine this randomly
        RoomType newType = currentRoom->getNeighbor(0);

        //Generate the new Room
        unique_ptr<Room> newRoom = roomFactory->createRoom(newType);

        //Transfer ownership of the newRoom to currentRoom. newRoom, now owning the previous currentRoom, is left to go out of scope.
        std::swap(newRoom, currentRoom);
    }

private:
    IRoomFactory* roomFactory;
    unique_ptr<Room> currentRoom;
};

class GameDataManager{
public:
    GameDataManager(Player* player, GameMap* map): player(player), map(map) {}

    Player* getPlayer() { return player; }

    GameMap* getMap() { return map; }

    void moveToNextRoom() { return map->moveToNextRoom(); }
private:
    Player* player;
    GameMap* map;
};

class IRenderer: public IRoomVisitor{
public:
    virtual ~IRenderer() = default;
    virtual void render() = 0;
    virtual void visit(EnemyRoom& enemyRoom) = 0;
private:
};

class TextRenderer : public IRenderer{
public:
    TextRenderer(GameDataManager* gameDataPtr): gameDataPtr(gameDataPtr){}

    void render() override {
        Player* player = gameDataPtr->getPlayer();
        GameMap* map = gameDataPtr->getMap();

        cout << "\n" << player->getName() << ":\n"; 
        cout << "Current HP: " << player->getHp() << "\n";
        cout << "Current Speed: " << player->getSpeed() << "\n";
        cout << "Current Strength: " << player->getStrength() << "\n";
        cout << "Current Agility: " << player->getAgility() << "\n";

        /*
        Will result in the visit() function of this class being called, 
        calling the specific overload for the room's derived class, 
        thus rendering the room correctly regardless of room type
        */
        map->currentRoomAcceptVisitor(*this); 
    }

    //Render the enemyRoom
    void visit(EnemyRoom& enemyRoom){
        Enemy* enemy = enemyRoom.getEnemy();
        cout << "\nA " << enemy->getName() << " bars your way! It has " << enemy->getHp() << " HP remaining.\n";
    }
private:
    GameDataManager* gameDataPtr;
};

class AsciiRenderer : public IRenderer{
public:
//TODO: implement renderGameData
    void render() override {
        cout << "Rendering game data as ascii\n";
    }
private:
};

class IActionHandler: public IRoomVisitor{
public:
    virtual ~IActionHandler() = default;

    //Diplay options and take input for those options.
    virtual void visit(EnemyRoom& enemyRoom) = 0;

    //Display actions and take input for them, depending on room type.
    virtual void startActionHandling(GameMap* gameMap) = 0;
};

class GameplayManager{
public:
    GameplayManager(unique_ptr<IRenderer> renderer, unique_ptr<GameDataManager> gameDataManager, unique_ptr<IActionHandler> actionHandler) : renderer(std::move(renderer)), gameDataManager(std::move(gameDataManager)), actionHandler(std::move(actionHandler)) {}
    void runGame(){
        GameplayStatus status = GameplayStatus::Ongoing;

        while(status == GameplayStatus::Ongoing){
            renderer->render();

            actionHandler->startActionHandling(gameDataManager->getMap());

            if(gameDataManager->getMap()->isCurrentRoomCleared()){
                cout << "\nThe room is clear now. Venture further in?\n";
                cout << "Quit: q\nContinue: any other\n";
                cout << "Your response: ";
                char response;
                cin >> response;

                if(response == 'q'){
                    status = GameplayStatus::Gameover;
                }
                else{
                    gameDataManager->getMap()->moveToNextRoom();
                }
            }
        }
    }
private:
    unique_ptr<IRenderer> renderer;
    unique_ptr<GameDataManager> gameDataManager;
    unique_ptr<IActionHandler> actionHandler;
};

class ConsoleActionHandler: public IActionHandler{
public:
    ConsoleActionHandler(GameDataManager* gameDataManager): gameDataManager(gameDataManager){}

    void startActionHandling(GameMap* gameMap){
        gameMap->currentRoomAcceptVisitor(*this);
    }

    void visit(EnemyRoom& enemyRoom){
        Enemy* enemy = enemyRoom.getEnemy();
        executePlayerCombatTurn(enemyRoom);
        
        if(enemyRoom.isCleared()){
            cout << "\nYou defeated the " << enemyRoom.getEnemy()->getName() << "!\n";
        }
        else{
            cout << "\nThe " << enemy->getName() << " is still standing at " << enemy->getHp() << " HP. It prepares to attack!\n";
            cout << "Input any key to continue... ";
            string throwaway;
            cin >> throwaway;
            executeEnemyCombatTurn(enemyRoom);
        }
    }
private:
    GameDataManager* gameDataManager;

    void executePlayerCombatTurn(EnemyRoom& enemyRoom){
        cout << "\n\n-------- Player Turn --------\n";
        Inventory* playerInventory = gameDataManager->getPlayer()->getInventory();
        vector<IWeapon*> weapons = playerInventory->getWeapons();
        vector<IActiveBuff*> activeBuffs = playerInventory->getActiveBuffs();

        cout << "\n";
        cout << "What will you do?\n";
        
        //Keep track of the option number
        int optionNumber = 1;

        //Display weapon actions first, and then buff actions
        for(IWeapon* weapon: weapons){
            cout << optionNumber << ": Attack (" << weapon->getName() << ")\n";
            optionNumber++;
        }
        for(IActiveBuff* buff: activeBuffs){
            cout << optionNumber << ": " << buff->getBuffAction() << " (" << buff->getName() << ")\n";
            optionNumber++;
        }

        //optionNumber has been incremented one additional time and now equals one more than the last valid option
        int lastValidOption = optionNumber - 1;

        cout << "\nInput your selection (1 - " << lastValidOption << ") ";

        //TODO: Add a quit option to this.
        int optionSelected = -1;
        bool initialLoopCompleted = false;
        while(optionSelected < 1 || optionSelected > lastValidOption){
            if(initialLoopCompleted){
                cout << "\nEnter a number between 1 and " << lastValidOption << ".";
            }
            
            cin >> optionSelected;

            if(cin.fail()){
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            initialLoopCompleted = true;
        }

        //Reset optionNumber and loop through the items again until option number has been incremented back up to
        //the chosen option, and execute the action for the item corresponging to that option.
        optionNumber = 1;
        IItem* itemUsed = nullptr; //Relevant for things like single use items that will be handled later.
        for(IWeapon* weapon: weapons){
            if(optionNumber == optionSelected){
                itemUsed = weapon;
                weapon->applyDamage(*gameDataManager->getPlayer(), *enemyRoom.getEnemy());
            }

            optionNumber++;
        }

        if(itemUsed == nullptr){
            for(IActiveBuff* buff: activeBuffs){
                if(optionNumber == optionSelected){
                    itemUsed = buff;
                    buff->applyBuff(*gameDataManager->getPlayer());
                }

                optionNumber++;
            }
        }

        //We need to remove the item from the inventory if it is marked as single use.
        if(itemUsed != nullptr && itemUsed->isSingleUse()){
            playerInventory->removeItem(itemUsed);
        }

        cout << "\n-------- End Player Turn --------\n\n";
    }

    void executeEnemyCombatTurn(EnemyRoom& enemyRoom){
        cout << "\n\n-------- Enemy Turn --------\n";

        //We need the enemy's potential actions, which is just their inventory items.
        Enemy* enemy = enemyRoom.getEnemy();
        Inventory* enemyInventory = enemy->getInventory();
        int potentialItemsToUse = enemyInventory->getTotalItems();

        //Now pick the specific action to use. TODO: Make this random.
        int itemToUse = 1;

        //Find the specified item and execute its action.
        vector<IWeapon*> weapons = enemyInventory->getWeapons();
        vector<IActiveBuff*> activeBuffs = enemyInventory->getActiveBuffs();
        int currentItemNumber = 1;
        IItem* itemUsed = nullptr; //Needed for if something happens to the item upon use, such as with single use items.
        for(IWeapon* weapon: weapons){
            if(currentItemNumber == itemToUse){
                cout << "\nThe " << enemy->getName() << " attacks with its " << weapon->getName() << " for " << weapon->getExpectedDamage(*enemy) << " damage.\n";
                weapon->applyDamage(*enemy, *gameDataManager->getPlayer());
                itemUsed = weapon;
            }

            currentItemNumber++;
        }

        if(itemUsed == nullptr){
            for(IActiveBuff* buff: activeBuffs){
                if(currentItemNumber == itemToUse){
                    cout << "\nThe " << enemy->getName() << " uses its " << buff->getName() << " to " << buff->getBuffAction() << " for " << buff->getMagnitude() << " points.\n";
                    buff->applyBuff(*enemy);
                    itemUsed = buff;
                }
            }

            currentItemNumber++;
        }

        //We need to remove the item from the inventory if it is marked as single use.
        if(itemUsed != nullptr && itemUsed->isSingleUse()){
            enemyInventory->removeItem(itemUsed);
        }

        cout << "\n-------- End Enemy Turn --------\n\n";
    }
};


int main(){
    //Initialize the Player
    unique_ptr<IItemFactory> itemFactory = make_unique<PredefinedItemFactory>();
    unique_ptr<IActorFactory> actorFactory = make_unique<PredefinedActorFactory>(itemFactory.get());
    unique_ptr<Player> player = actorFactory->createPlayer();

    //Initialize the Map
    unique_ptr<IRoomFactory> roomFactory = make_unique<PredefinedRoomFactory>(actorFactory.get());
    unique_ptr<GameMap> map = make_unique<GameMap>(roomFactory.get());

    //Initilize the game data and renderer
    unique_ptr<GameDataManager> gameDataManager = make_unique<GameDataManager>(player.get(), map.get());
    unique_ptr<IRenderer> renderer = make_unique<TextRenderer>(gameDataManager.get());

    //Initialize the ActionHandler
    unique_ptr<IActionHandler> actionHandler = make_unique<ConsoleActionHandler>(gameDataManager.get());

    //Initialize the GameplayManager and start the game
    unique_ptr<GameplayManager> gameplayManager = make_unique<GameplayManager>(std::move(renderer), std::move(gameDataManager), std::move(actionHandler));
    gameplayManager->runGame();

    return 0;
}
