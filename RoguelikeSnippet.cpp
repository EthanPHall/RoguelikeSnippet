#include <iostream>
#include <string>
#include <memory>

using std::cout;
using std::cin;
using std::string;
using std::array;
using std::unique_ptr;
using std::make_unique;
// using std::move; // I choose not to use this specific name because "move" is such generic word and I don't want to potentially run into an ambiguous names issue.

class Room; //Base class for all rooms
class IRoomFactory; 
class PredefinedRoomFactory;//Implementation of IRoomFactory. Returns Room objects defined by the factory itself based on provided RoomTypes
enum class RoomType { Enemy };
class EnemyRoom; //Spawns in an enemy. The player must defeat the the enemy to move on, or game over.
class GameMap; //Handles generating the next room, keeping track of the current room, and getting data from the current room.

class Actor; //Base class for all Actors, objects that simulate characters, creatures, and any other entity that has agency in how it acts
class IActorFactory; //Interface for Actor factories
class PredefinedActorFactory; //Actor Factory implementation that creates and returns actors defined by the factory itself
enum class EnemyType { Goblin };
class Enemy; 
class Player;

class IItem;
class ItemFactory;
enum class ItemType { Sword, HealthPotion };
class IWeapon;
class IActiveBuff;
class Sword;
class HealthPotion;

class GameplayManager; //Runs the main game loop, takes in input, and keep track of progress and whether or not to move on, or end the game.
enum class GameplayStatus {Ongoing, Victory, Gameover};
class GameDataManager;
class IRenderer;
class TextRenderer;
class AsciiRenderer;



class Actor{
public:
    Actor(string name, int hp, int strength, int speed, int agility) : name(name), hp(hp), strength(strength), speed(speed), agility(agility) {}
    string getName() { return name; }
    int getHp() { return hp; }
    int getStrength() { return strength; }
    int getSpeed() { return speed; }
    int getAgility() { return agility; }
private:
    string name;
    int hp;
    int strength;
    int speed;
    int agility;
};

class Enemy : public Actor{
public:
    Enemy(string name, int hp, int strength, int speed, int agility) : Actor(name, hp, strength, speed, agility) {}
private:
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
                return std::move(make_unique<Enemy>("Goblin", 10, 1, 1, 2));
            }
            default:
            {    
                return std::move(make_unique<Enemy>("Goblin", 10, 1, 1, 2));
            }
        }
    }

    unique_ptr<Player> createPlayer() const{
        return std::move(make_unique<Player>("Player", 100, 1, 1, 1));
    }
};



class Room{
public:
    Room(string name, RoomType type, array<RoomType, 3> neighbors) : name(name), type(type), neighbors(neighbors) {}
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

class EnemyRoom: public Room{
public:
    EnemyRoom(string name, RoomType type, array<RoomType, 3> neighbors, const IActorFactory* actorFactory): Room(name, type, neighbors), actorFactory(actorFactory) {
        enemy = std::move(actorFactory->createEnemy(EnemyType::Goblin));
    }
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



class IItem{
public:
    virtual string getName() = 0;
private:
};

class IWeapon : public IItem{
public:
    virtual int applyDamage(unique_ptr<Actor> target, int damageModifier) = 0;
private:
};

class IActiveBuff : public IItem{
public:
    virtual void applyBuff(unique_ptr<Actor> target) = 0;
private:
};

class Weapon : public IWeapon{
public:
    Weapon(string name, int damage) : name(name), damage(damage) {}
private:
    string name;
    int damage;
};

class HealthPotion : public IActiveBuff{
public:
    HealthPotion(string name, int healAmount) : name(name), healAmount(healAmount) {}
private:
    string name;
    int healAmount;
};



class GameDataManager{
public:
    GameDataManager(Player* player): player(player) {}
    int getPlayerHp() { return player->getHp(); }
    string getPlayerName() { return player->getName(); }
    int getPlayerSpeed() { return player->getSpeed(); }
    int getPlayerStrength() { return player->getStrength(); }
    int getPlayerAgility() { return player->getAgility(); }
private:
    Player* player;
};

class IRenderer{
public:
    virtual void renderGameData(GameDataManager* gameDataPtr) = 0;
    virtual ~IRenderer() = default;
private:
};

class TextRenderer : public IRenderer{
public:
//TODO: implement renderGameData
    void renderGameData(GameDataManager* gameDataPtr) override {
        cout << gameDataPtr->getPlayerName() << ":\n"; 
        cout << "Current HP: " << gameDataPtr->getPlayerHp() << "\n";
        cout << "Current Speed: " << gameDataPtr->getPlayerSpeed() << "\n";
        cout << "Current Strength: " << gameDataPtr->getPlayerStrength() << "\n";
        cout << "Current Agility: " << gameDataPtr->getPlayerAgility() << "\n";
        cout << "\n";
    }
private:
};

class AsciiRenderer : public IRenderer{
public:
//TODO: implement renderGameData
    void renderGameData(GameDataManager* gameDataPtr) override {
        cout << "Rendering game data as ascii\n";
    }
private:
};

class GameplayManager{
public:
    GameplayManager(unique_ptr<IRenderer> renderer, unique_ptr<GameDataManager> gameDataManager) : renderer(std::move(renderer)), gameDataManager(std::move(gameDataManager)) {}
    void runGame(){
        GameplayStatus status = GameplayStatus::Ongoing;

        while(status == GameplayStatus::Ongoing){
            renderer->renderGameData(gameDataManager.get());
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
};



int main(){
    //Initialize the Player
    unique_ptr<IActorFactory> actorFactory = make_unique<PredefinedActorFactory>();
    unique_ptr<Player> player = actorFactory->createPlayer();

    //Initialize the Map
    unique_ptr<IRoomFactory> roomFactory = make_unique<PredefinedRoomFactory>(actorFactory.get());
    unique_ptr<GameMap> map = make_unique<GameMap>(roomFactory.get());

    //Initilize the game data and renderer
    unique_ptr<IRenderer> renderer = make_unique<TextRenderer>();
    unique_ptr<GameDataManager> gameDataManager = make_unique<GameDataManager>(player.get());

    //Initialize the GameplayManager and start the game
    unique_ptr<GameplayManager> gameplayManager = make_unique<GameplayManager>(std::move(renderer), std::move(gameDataManager));
    gameplayManager->runGame();

    return 0;
}
