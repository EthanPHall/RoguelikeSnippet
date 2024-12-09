#include <iostream>
#include <string>
#include <memory>

using std::cout;
using std::cin;
using std::string;
using std::array;

class Room;

class Actor;
class Enemy;
class Player;

//I use the PA prefix to indicate that this is a pure abstract class
class PAItem;

int main(){



}

class Room{
public:
private:
    string name;
    //neighbors are the rooms that can be visited from this room, so the previous room is not included
    array<Room*, 3> neighbors;
};

class Actor{
public:
private:
    string name;
    int hp;
};

class Enemy : public Actor{
public:
private:
};

class Player : public Actor{
public:
private:
};