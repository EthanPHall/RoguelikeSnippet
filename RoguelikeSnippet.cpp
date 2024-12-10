#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <limits>

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

class PredefinedActorFactory: public IActorFactory{
public:
    unique_ptr<Enemy> createEnemy(EnemyType type) const{
        switch(type){
            case EnemyType::Goblin:
            {
                //TODO: Make this create a Goblin
                return std::move(make_unique<Enemy>("Goblin", 10, 1, 1, 2, EnemyType::Goblin));
            }
            default:
            {    
                return std::move(make_unique<Enemy>("Goblin", 10, 1, 1, 2, EnemyType::Goblin));
            }
        }
    }

    unique_ptr<Player> createPlayer() const{
        unique_ptr<Player> newPlayer = make_unique<Player>("Player", 100, 1, 1, 1);

        return std::move(newPlayer);
    }
};





class IItem{
public:
    virtual string getName() = 0;
private:
};

class IWeapon : public IItem{
public:
    virtual void applyDamage(Actor& user, Actor& target) = 0;
    virtual string getName() = 0;
private:
};

class IActiveBuff : public IItem{
public:
    virtual void applyBuff(Actor& target) = 0;
    virtual string getName() = 0;
    virtual string getBuffAction() = 0;
private:
};

class Sword : public IWeapon{
public:
    Sword(string name, int damage) : name(name), damage(damage) {}
    string getName(){ return name; }
    void applyDamage(Actor& user, Actor& target){
        int damageToDeal = damage + user.getAgility() + user.getStrength();
    }
private:
    string name;
    int damage;
};

class HealthPotion : public IActiveBuff{
public:
    HealthPotion(string name, int healAmount) : name(name), healAmount(healAmount) {}
    string getName(){
        return name;
    }
    string getBuffAction(){
        return "Heal";
    }
    void applyBuff(Actor& target){
        target.modHp(healAmount);
    }
private:
    string name;
    int healAmount;
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
private:
    vector<unique_ptr<IWeapon>> weapons;
    vector<unique_ptr<IActiveBuff>> activeBuffs;
};

class IItemFactory{
public:
    virtual ~IItemFactory() = default;
    virtual void createAndStoreItem(Inventory* toStoreIn, ItemType type) = 0;
private:
};

class PredefinedItemFactory: public IItemFactory{
public:
    void createAndStoreItem(Inventory* toStoreIn, ItemType type){
        switch(type){
            case ItemType::Sword:{
                unique_ptr<Sword> newSword = make_unique<Sword>("Sword", 3);
                toStoreIn->transferWeaponOwnership(std::move(newSword));
            }
            case ItemType::HealthPotion:{
                unique_ptr<HealthPotion> newPotion = make_unique<HealthPotion>("Health Potion", 20);
                toStoreIn->transferActiveBuffOwnership(std::move(newPotion));
            }
        }
    }
private:
};





class Room{
public:
    Room(string name, RoomType type, array<RoomType, 3> neighbors) : name(name), type(type), neighbors(neighbors) {}
    virtual void accept(IRoomVisitor& visitor) = 0;
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

    void accept(IRoomVisitor& visitor){
        visitor.visit(*this);
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

        cout << player->getName() << ":\n"; 
        cout << "Current HP: " << player->getHp() << "\n";
        cout << "Current Speed: " << player->getSpeed() << "\n";
        cout << "Current Strength: " << player->getStrength() << "\n";
        cout << "Current Agility: " << player->getAgility() << "\n";
        cout << "\n\n";

        /*
        Will result in the visit() function of this class being called, 
        calling the specific overload for the room's derived class, 
        thus rendering the room correctly regardless of room type
        */
        map->currentRoomAcceptVisitor(*this); 
        cout << "\n\n";
    }

    //Render the enemyRoom
    void visit(EnemyRoom& enemyRoom){
        Enemy* enemy = enemyRoom.getEnemy();
        cout << "A " << enemy->getName() << " bars your way! It has " << enemy->getHp() << " HP remaining.";
    }
private:
    void renderRoom(GameDataManager* gameDataPtr){
    }

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

class GameplayManager{
public:
    GameplayManager(unique_ptr<IRenderer> renderer, unique_ptr<GameDataManager> gameDataManager, unique_ptr<IActionHandler> actionHandler) : renderer(std::move(renderer)), gameDataManager(std::move(gameDataManager)), actionHandler(std::move(actionHandler)) {}
    void runGame(){
        GameplayStatus status = GameplayStatus::Ongoing;

        while(status == GameplayStatus::Ongoing){
            renderer->render();

            //Display actions and take input for them, depending on room type.
            // gameDataManager->getMap()->currentRoomAcceptVisitor(*actionHandler.get());

            cout << "\n";
            cout << "Quit: q\nContinue: any other\n";
            cout << "Your response: ";
            char response;
            cin >> response;

            if(response == 'q'){
                status = GameplayStatus::Gameover;
            }
        }
    }
private:
    unique_ptr<IRenderer> renderer;
    unique_ptr<GameDataManager> gameDataManager;
    unique_ptr<IActionHandler> actionHandler;
};

class IActionHandler: public IRoomVisitor{
public:
    virtual ~IActionHandler() = default;

    //Diplay options and take input for those options.
    virtual void visit(EnemyRoom& enemyRoom) = 0;
};

class ConsoleActionHandler: public IActionHandler{
public:
    ConsoleActionHandler(GameDataManager* gameDataManager): gameDataManager(gameDataManager){}
    void visit(EnemyRoom& enemyRoom){
        Inventory* playerInventory = gameDataManager->getPlayer()->getInventory();
        vector<IWeapon*> weapons = playerInventory->getWeapons();
        vector<IActiveBuff*> activeBuffs = playerInventory->getActiveBuffs();

        cout << "\n";
        cout << "What will you do?\n";
        
        //Keep track of the option number
        int optionNumber = 1;

        //Display weapon actions first, and then buff actions
        for(IWeapon* weapon: weapons){
            cout << optionNumber << ": Attack (" << weapon->getName() << ")";
            optionNumber++;
        }
        for(IActiveBuff* buff: activeBuffs){
            cout << optionNumber << ": " << buff->getBuffAction() << " (" << buff->getName() << ")";
            optionNumber++;
        }

        //optionNumber has been incremented and now represents the last valid option
        cout << "\n\nInput your selection (1 - " << optionNumber << ") ";

        //TODO: Add a quit option to this.
        int optionSelected = -1;
        bool initialLoopCompleted = false;
        while(optionSelected < 1 || optionSelected > optionNumber){
            if(initialLoopCompleted){
                cout << "\nEnter a number between 1 and " << optionNumber << ".";
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
        bool optionFound = false;
        for(IWeapon* weapon: weapons){
            if(optionNumber == optionSelected){
                optionFound = true;
                weapon->applyDamage(*gameDataManager->getPlayer(), *enemyRoom.getEnemy());
            }

            optionNumber++;
        }

        if(!optionFound){
            for(IActiveBuff* buff: activeBuffs){
                if(optionNumber == optionSelected){
                    optionFound = true;
                    buff->applyBuff(*gameDataManager->getPlayer());
                }

                optionNumber++;
            }
        }
    }
private:
    GameDataManager* gameDataManager;
};


int main(){
    //Initialize the Player
    unique_ptr<IActorFactory> actorFactory = make_unique<PredefinedActorFactory>();
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
